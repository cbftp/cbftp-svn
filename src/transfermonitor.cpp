#include "transfermonitor.h"

#include "core/tickpoke.h"
#include "site.h"
#include "sitelogic.h"
#include "siterace.h"
#include "ftpconn.h"
#include "filelist.h"
#include "globalcontext.h"
#include "file.h"
#include "localstorage.h"
#include "localfilelist.h"
#include "localfile.h"
#include "transfermanager.h"
#include "transferstatus.h"
#include "localtransfer.h"
#include "localdownload.h"
#include "filesystem.h"
#include "util.h"

TransferMonitor::TransferMonitor(TransferManager * tm) :
  status(TM_STATUS_IDLE),
  clientactive(true),
  fxpdstactive(true),
  timestamp(0),
  tm(tm),
  localtransferspeedticker(0),
  checkdeadticker(0)
{
  global->getTickPoke()->startPoke(this, "TransferMonitor", TICKINTERVAL, 0);
}

TransferMonitor::~TransferMonitor() {

}

bool TransferMonitor::idle() const {
  return status == TM_STATUS_IDLE;
}

void TransferMonitor::engageFXP(std::string sfile, SiteLogic * sls, FileList * fls, std::string dfile, SiteLogic * sld, FileList * fld) {
  reset();
  type = TM_TYPE_FXP;
  this->sls = sls;
  this->sld = sld;
  this->fls = fls;
  this->spath = fls->getPath();
  this->fld = fld;
  this->dpath = fld->getPath();
  this->sfile = sfile;
  this->dfile = dfile;
  if (!sls->lockDownloadConn(spath, &src, this)) {
    tm->transferFailed(ts, TM_ERR_LOCK_DOWN);
    return;
  }
  if (!sld->lockUploadConn(dpath, &dst, this)) {
    sls->returnConn(src);
    tm->transferFailed(ts, TM_ERR_LOCK_UP);
    return;
  }
  status = TM_STATUS_AWAITING_PASSIVE;
  int spol = sls->getSite()->getSSLTransferPolicy();
  int dpol = sld->getSite()->getSSLTransferPolicy();
  if (spol != SITE_SSL_ALWAYS_OFF && dpol != SITE_SSL_ALWAYS_OFF &&
      (spol == SITE_SSL_ALWAYS_ON || dpol == SITE_SSL_ALWAYS_ON ||
      (spol == SITE_SSL_PREFER_ON && dpol == SITE_SSL_PREFER_ON))) {
    ssl = true;
  }
  fld->touchFile(dfile, sld->getSite()->getUser(), true);
  latesttouch = fld->getFile(dfile)->getTouch();
  fls->download(sfile);
  fxpdstactive = !sls->getSite()->hasBrokenPASV();
  ts = makePointer<TransferStatus>(TRANSFERSTATUS_TYPE_FXP, sls->getSite()->getName(),
      sld->getSite()->getName(), "", dfile, fls, fls->getPath(), fld, fld->getPath(),
      fls->getFile(sfile)->getSize(),
      sls->getSite()->getAverageSpeed(sld->getSite()->getName()), src, dst, ssl, fxpdstactive);
  tm->addNewTransferStatus(ts);
  if (fxpdstactive) {
    sls->preparePassiveTransfer(src, spath, sfile, true, ssl);
  }
  else {
    sld->preparePassiveTransfer(dst, dpath, dfile, true, ssl);
  }


}

void TransferMonitor::engageDownload(std::string sfile, SiteLogic * sls, FileList * fls, Pointer<LocalFileList> & localfl) {
  reset();
  if (!localfl) return;
  type = TM_TYPE_DOWNLOAD;
  this->sls = sls;
  this->sfile = sfile;
  this->dfile = sfile;
  this->spath = fls->getPath();
  this->dpath = localfl->getPath();
  this->fls = fls;
  this->localfl = localfl;
  if (!sls->lockDownloadConn(spath, &src, this)) return;
  status = TM_STATUS_AWAITING_PASSIVE;
  int spol = sls->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  localfl->touchFile(dfile);
  if (!FileSystem::directoryExistsWritable(dpath)) {
    FileSystem::createDirectoryRecursive(dpath);
  }
  clientactive = !sls->getSite()->hasBrokenPASV();
  ts = makePointer<TransferStatus>(TRANSFERSTATUS_TYPE_DOWNLOAD,
      sls->getSite()->getName(), "/\\", "", dfile, fls, spath,
      (FileList *)NULL, dpath, fls->getFile(sfile)->getSize(), 0, src, -1, ssl, clientactive);
  tm->addNewTransferStatus(ts);
  if (clientactive) {
    sls->preparePassiveTransfer(src, spath, sfile, false, ssl);
  }
  else {
    lt = global->getLocalStorage()->activeModeDownload(this, dpath, dfile, ssl, sls->getConn(src));
    passiveReady(global->getLocalStorage()->getAddress(lt), lt->getPort());
  }
}

void TransferMonitor::engageUpload(std::string sfile, Pointer<LocalFileList> & localfl, SiteLogic * sld, FileList * fld) {
  reset();
  if (!localfl) return;
  type = TM_TYPE_UPLOAD;
  this->sld = sld;
  this->sfile = sfile;
  this->dfile = sfile;
  this->spath = localfl->getPath();
  this->dpath = fld->getPath();
  this->fld = fld;
  this->localfl = localfl;
  std::map<std::string, LocalFile>::const_iterator it = localfl->find(sfile);
  if (it == localfl->end()) return;
  const LocalFile & lf = it->second;
  if (!sld->lockUploadConn(dpath, &dst, this)) return;
  status = TM_STATUS_AWAITING_PASSIVE;
  int spol = sld->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  fld->touchFile(dfile, sld->getSite()->getUser(), true);
  clientactive = !sld->getSite()->hasBrokenPASV();
  ts = makePointer<TransferStatus>(TRANSFERSTATUS_TYPE_UPLOAD,
      "/\\", sld->getSite()->getName(), "", dfile, (FileList *)NULL, spath,
      fld, dpath, lf.getSize(), 0, 0, dst, ssl, clientactive);
  tm->addNewTransferStatus(ts);
  if (clientactive) {
    sld->preparePassiveTransfer(dst, dpath, dfile, false, ssl);
  }
  else {
    lt = global->getLocalStorage()->activeModeUpload(this, spath, sfile, ssl, sld->getConn(dst));
    passiveReady(global->getLocalStorage()->getAddress(lt), lt->getPort());
  }
}

void TransferMonitor::engageList(SiteLogic * sls, int connid, bool hiddenfiles) {
  reset();
  type = TM_TYPE_LIST;
  this->sls = sls;
  src = connid;
  this->hiddenfiles = hiddenfiles;
  int spol = sls->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  status = TM_STATUS_AWAITING_PASSIVE;
  if (!sls->getSite()->hasBrokenPASV()) {
    sls->preparePassiveList(src, this, ssl);
  }
  else {
    clientactive = false;
    lt = global->getLocalStorage()->activeModeDownload(this, ssl, sls->getConn(src));
    storeid = static_cast<LocalDownload *>(lt)->getStoreId();
    passiveReady(global->getLocalStorage()->getAddress(lt), lt->getPort());
  }
}

void TransferMonitor::tick(int msg) {
  if (status != TM_STATUS_IDLE) {
    timestamp += TICKINTERVAL;
    if (type == TM_TYPE_FXP) {
      updateFXPSizeSpeed();
      if (checkdeadticker++ % 20 == 0) { // run once per second
        checkForDeadFXPTransfers();
      }
    }
    if (type == TM_TYPE_DOWNLOAD || type == TM_TYPE_UPLOAD) {
      if (localtransferspeedticker++ % 4 == 0) { // run every 200 ms
        updateLocalTransferSizeSpeed();
      }
    }
  }
}

void TransferMonitor::passiveReady(const std::string & host, int port) {
  util::assert(status == TM_STATUS_AWAITING_PASSIVE ||
               status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
               status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET);
  if (status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE) {
    sourceError(TM_ERR_OTHER);
    sls->returnConn(src);
    return;
  }
  if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET) {
    targetError(TM_ERR_OTHER);
    sld->returnConn(dst);
    return;
  }
  status = TM_STATUS_AWAITING_ACTIVE;
  if (!!ts) {
    ts->setPassiveAddress(host + ":" + util::int2Str(port));
  }
  switch (type) {
    case TM_TYPE_FXP:
      if (fxpdstactive) {
        sld->prepareActiveTransfer(dst, dpath, dfile, host, port, ssl);
      }
      else {
        sls->prepareActiveTransfer(src, spath, sfile, host, port, ssl);
      }
      break;
    case TM_TYPE_DOWNLOAD:
      if (clientactive) {
        lt = global->getLocalStorage()->passiveModeDownload(this, dpath, dfile, host, port, ssl, sls->getConn(src));
      }
      else {
        sls->prepareActiveTransfer(src, spath, sfile, host, port, ssl);
      }
      break;
    case TM_TYPE_UPLOAD:
      if (clientactive) {
        lt = global->getLocalStorage()->passiveModeUpload(this, spath, sfile, host, port, ssl, sld->getConn(dst));
      }
      else {
        sld->prepareActiveTransfer(dst, dpath, dfile, host, port, ssl);
      }
      break;
    case TM_TYPE_LIST:
      if (clientactive) {
        lt = global->getLocalStorage()->passiveModeDownload(this, host, port, ssl, sls->getConn(src));
        storeid = static_cast<LocalDownload *>(lt)->getStoreId();
      }
      else {
        sls->prepareActiveList(src, this, host, port, ssl);
      }
      break;
  }
}

void TransferMonitor::activeReady() {
  util::assert(status == TM_STATUS_AWAITING_ACTIVE ||
               status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
               status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET);
  if (status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE) {
    sourceError(TM_ERR_OTHER);
    sls->returnConn(src);
    return;
  }
  if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET) {
    targetError(TM_ERR_OTHER);
    sld->returnConn(dst);
    return;
  }
  if (type == TM_TYPE_FXP) {
    if (fxpdstactive) {
      sld->upload(dst);
    }
    else {
      sls->download(src);
    }
  }
  else {
    startClientTransfer();
  }
}

void TransferMonitor::activeStarted() {
  util::assert(status == TM_STATUS_AWAITING_ACTIVE ||
               status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
               status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET);
  if (status == TM_STATUS_AWAITING_ACTIVE) {
    status = TM_STATUS_TRANSFERRING;
    if (type == TM_TYPE_FXP) {
      if (fxpdstactive) {
        sls->download(src);
      }
      else {
        sld->upload(dst);
      }
    }
    else if (clientactive) {
      startClientTransfer();
    }
    startstamp = timestamp;
  }
}

void TransferMonitor::cipher(const std::string & cipher) {
  ts->setCipher(cipher);
}

void TransferMonitor::startClientTransfer() {
  switch (type) {
    case TM_TYPE_UPLOAD:
      sld->upload(dst);
      break;
    case TM_TYPE_DOWNLOAD:
      sls->download(src);
      break;
    case TM_TYPE_LIST:
      if (hiddenfiles) {
        sls->listAll(src);
      }
      else {
        sls->list(src);
      }
      break;
    case TM_TYPE_FXP:
      util::assert(false);
      break;
  }
}

void TransferMonitor::sourceComplete() {
  util::assert(status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
               status == TM_STATUS_TRANSFERRING ||
               status == TM_STATUS_TRANSFERRING_TARGET_COMPLETE);
  partialcompletestamp = timestamp;
  if (fls != NULL) {
    fls->finishDownload(sfile);
  }
  if (status == TM_STATUS_TRANSFERRING) {
    status = TM_STATUS_TRANSFERRING_SOURCE_COMPLETE;
  }
  else {
    finish(status != TM_STATUS_TARGET_ERROR_AWAITING_SOURCE);
  }
}

void TransferMonitor::targetComplete() {
  util::assert(status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
               status == TM_STATUS_TRANSFERRING ||
               status == TM_STATUS_TRANSFERRING_SOURCE_COMPLETE);
  partialcompletestamp = timestamp;
  if (fld != NULL) {
    fld->finishUpload(dfile);
  }
  if (status == TM_STATUS_TRANSFERRING) {
    status = TM_STATUS_TRANSFERRING_TARGET_COMPLETE;
  }
  else {
    finish(status != TM_STATUS_SOURCE_ERROR_AWAITING_TARGET);
  }
}

void TransferMonitor::finish(bool successful) {
  if (successful) {
    int span = timestamp - startstamp;
    if (span == 0) {
      span = 10;
    }
    switch (type) {
      case TM_TYPE_FXP:
      case TM_TYPE_DOWNLOAD: {
        File * srcfile = fls->getFile(sfile);
        if (srcfile) {
          unsigned long long int size = srcfile->getSize();
          unsigned int speed = size / span;
          ts->setTargetSize(size);
          ts->setSpeed(speed);
          ts->setTimeSpent(span / 1000);
          if (size > 1000000 && type == TM_TYPE_FXP) {
            fld->setFileUpdateFlag(dfile, size, speed, sls->getSite(), sld->getSite()->getName());
          }
        }
        break;
      }
      case TM_TYPE_UPLOAD: {
        unsigned long long int size = lt->size();
        unsigned int speed = size / span;
        ts->setTargetSize(size);
        ts->setSpeed(speed);
        ts->setTimeSpent(span / 1000);
        break;
      }
      case TM_TYPE_LIST:
        sls->listCompleted(src, storeid);
        break;
    }
    if (!!ts) {
      ts->setFinished();
    }
    tm->transferSuccessful(ts);
  }
  else {
    transferFailed(ts, TM_ERR_OTHER);
    return;
  }
  status = TM_STATUS_IDLE;
}

void TransferMonitor::sourceError(TransferError err) {
  util::assert(status == TM_STATUS_AWAITING_ACTIVE ||
               status == TM_STATUS_AWAITING_PASSIVE ||
               status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
               status == TM_STATUS_TRANSFERRING ||
               status == TM_STATUS_TRANSFERRING_TARGET_COMPLETE);
  if (fls != NULL) {
    fls->finishDownload(sfile);
  }
  partialcompletestamp = timestamp;
  if (status == TM_STATUS_AWAITING_PASSIVE || status == TM_STATUS_AWAITING_ACTIVE) {
    if (sld != NULL && !sld->getConn(dst)->isProcessing()) {
      sld->returnConn(dst);
      fld->finishUpload(dfile);
      transferFailed(ts, err);
      return;
    }
  }
  if (status == TM_STATUS_TRANSFERRING_TARGET_COMPLETE ||
      status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE ||
      (type == TM_TYPE_DOWNLOAD && lt == NULL))
  {
    transferFailed(ts, err);
    return;
  }
  status = TM_STATUS_SOURCE_ERROR_AWAITING_TARGET;
}

void TransferMonitor::targetError(TransferError err) {
  util::assert(status == TM_STATUS_AWAITING_ACTIVE ||
               status == TM_STATUS_AWAITING_PASSIVE ||
               status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
               status == TM_STATUS_TRANSFERRING ||
               status == TM_STATUS_TRANSFERRING_SOURCE_COMPLETE);
  if (fld != NULL) {
    fld->finishUpload(dfile);
  }
  partialcompletestamp = timestamp;
  if (status == TM_STATUS_AWAITING_PASSIVE || status == TM_STATUS_AWAITING_ACTIVE) {
    if (sls != NULL && !sls->getConn(src)->isProcessing()) {
      sls->returnConn(src);
      if (fls != NULL) { // NULL in case of LIST
        fls->finishDownload(sfile);
      }
      transferFailed(ts, err);
      return;
    }
  }
  if (status == TM_STATUS_TRANSFERRING_SOURCE_COMPLETE ||
      status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
      (type == TM_TYPE_UPLOAD && lt == NULL))
  {
    transferFailed(ts, err);
    return;
  }
  status = TM_STATUS_TARGET_ERROR_AWAITING_SOURCE;
}

Pointer<TransferStatus> TransferMonitor::getTransferStatus() const {
  return ts;
}

void TransferMonitor::updateFXPSizeSpeed() {
  File * file = fld->getFile(dfile);
  if (file) {
    unsigned long long int filesize = file->getSize();
    int span = timestamp - startstamp;
    int touch = file->getTouch();
    if (!span) {
      span = 10;
    }

    // if the file list has been updated (as seen on the file's touch stamp
    // the speed shall be recalculated.
    if (latesttouch != touch) {
      latesttouch = touch;
      setTargetSizeSpeed(filesize, span);
    }
    else {
      // since the actual file size has not changed since last tick,
      // interpolate an updated file size through the currently known speed
      unsigned long long int speedtemp = ts->getSpeed() * 1024;
      ts->interpolateAddSize((speedtemp * TICKINTERVAL) / 1000);
    }
    ts->setTimeSpent(span / 1000);
  }
}

void TransferMonitor::updateLocalTransferSizeSpeed() {
  if (lt) {
    unsigned int filesize = lt->size();
    int span = timestamp - startstamp;
    if (!span) {
      span = 10;
    }
    setTargetSizeSpeed(filesize, span);
    ts->setTimeSpent(span / 1000);
  }
}

void TransferMonitor::checkForDeadFXPTransfers() {
  if ((status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET &&
       timestamp - partialcompletestamp > MAX_WAIT_ERROR) ||
      (status == TM_STATUS_TRANSFERRING_SOURCE_COMPLETE &&
       timestamp - partialcompletestamp > MAX_WAIT_SOURCE_COMPLETE))
  {
    sld->disconnectConn(dst);
    sld->connectConn(dst);
  }
  else if (status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE &&
           timestamp - partialcompletestamp > MAX_WAIT_ERROR)
  {
    sls->disconnectConn(src);
    sls->connectConn(src);
  }
  else if (status == TM_STATUS_TRANSFERRING_TARGET_COMPLETE &&
           timestamp - partialcompletestamp > MAX_WAIT_ERROR)
  {
    sls->finishTransferGracefully(src);
    sls->disconnectConn(src);
    sls->connectConn(src);
  }
}

void TransferMonitor::setTargetSizeSpeed(unsigned long long int filesize, int span) {
  ts->setTargetSize(filesize);
  unsigned int currentspeed = ts->targetSize() / span;
  unsigned int prevspeed = ts->getSpeed();
  if (currentspeed < prevspeed) {
    ts->setSpeed(prevspeed * 0.9 + currentspeed * 0.1);
  }
  else {
    ts->setSpeed(prevspeed * 0.7 + currentspeed * 0.3);
  }
}

Status TransferMonitor::getStatus() const {
  return status;
}

void TransferMonitor::reset() {
  sls = NULL;
  sld = NULL;
  fls = NULL;
  fld = NULL;
  lt = NULL;
  ts.reset();
  clientactive = true;
  fxpdstactive = true;
  ssl = false;
  timestamp = 0;
  startstamp = 0;
  partialcompletestamp = 0;
}

void TransferMonitor::transferFailed(Pointer<TransferStatus> & ts, TransferError err) {
  if (!!ts) {
    ts->setFailed();
  }
  if (type == TM_TYPE_FXP) {
    sls->getSite()->pushTransferSpeed(sld->getSite()->getName(), 0, 0);
  }
  tm->transferFailed(ts, err);
  status = TM_STATUS_IDLE;
}

void TransferMonitor::newRawBufferLine(const std::pair<std::string, std::string> & line) {
  if (!!ts) {
    ts->addLogLine(line.first + " " + line.second);
  }
}
