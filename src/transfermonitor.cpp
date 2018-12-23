#include "transfermonitor.h"

#include <algorithm>
#include <unordered_map>

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
#include "statistics.h"

#define MAX_WAIT_ERROR 10000
#define MAX_WAIT_SOURCE_COMPLETE 60000

#define TICKINTERVAL 50

TransferMonitor::TransferMonitor(TransferManager * tm) :
  status(TM_STATUS_IDLE),
  clientactive(true),
  fxpdstactive(true),
  timestamp(0),
  tm(tm),
  ticker(0)
{
  global->getTickPoke()->startPoke(this, "TransferMonitor", TICKINTERVAL, 0);
}

TransferMonitor::~TransferMonitor() {

}

bool TransferMonitor::idle() const {
  return status == TM_STATUS_IDLE;
}

void TransferMonitor::engageFXP(
  const std::string & sfile, const std::shared_ptr<SiteLogic> & sls, FileList * fls,
  const std::string & dfile, const std::shared_ptr<SiteLogic> & sld, FileList * fld,
  CommandOwner * srcco, CommandOwner * dstco)
{
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
  this->srcco = srcco;
  this->dstco = dstco;
  if (!sls->lockDownloadConn(fls, &src, srcco, this)) {
    tm->transferFailed(ts, TM_ERR_LOCK_DOWN);
    return;
  }
  if (!sld->lockUploadConn(fld, &dst, dstco, this)) {
    sls->returnConn(src, true);
    tm->transferFailed(ts, TM_ERR_LOCK_UP);
    return;
  }
  status = TM_STATUS_AWAITING_PASSIVE;
  fxpdstactive = !sls->getSite()->hasBrokenPASV();
  int spol = sls->getSite()->getSSLTransferPolicy();
  int dpol = sld->getSite()->getSSLTransferPolicy();
  if (spol != SITE_SSL_ALWAYS_OFF && dpol != SITE_SSL_ALWAYS_OFF &&
      (spol == SITE_SSL_ALWAYS_ON || dpol == SITE_SSL_ALWAYS_ON ||
      (spol == SITE_SSL_PREFER_ON && dpol == SITE_SSL_PREFER_ON)) &&
      (sls->getSite()->supportsSSCN() || sld->getSite()->supportsSSCN()))
  {
    ssl = true;
    sourcesslclient = fxpdstactive;
    if ((fxpdstactive && !sls->getSite()->supportsSSCN() && !sls->getSite()->supportsCPSV()) ||
        (!fxpdstactive && !sld->getSite()->supportsSSCN() && !sld->getSite()->supportsCPSV()))
    {
      sourcesslclient = !sourcesslclient;
    }
  }
  fld->touchFile(dfile, sld->getSite()->getUser(), true);
  latestsrctouch = fls->getFile(sfile)->getTouch();
  latestdsttouch = fld->getFile(dfile)->getTouch();
  fls->download(sfile);

  ts = std::make_shared<TransferStatus>(TRANSFERSTATUS_TYPE_FXP, sls->getSite()->getName(),
      sld->getSite()->getName(), "", dfile, fls, fls->getPath(), fld, fld->getPath(),
      fls->getFile(sfile)->getSize(),
      sls->getSite()->getAverageSpeed(sld->getSite()->getName()), src, dst, ssl, fxpdstactive);
  tm->addNewTransferStatus(ts);
  if (fxpdstactive) {
    sls->preparePassiveTransfer(src, sfile, true, ssl, sourcesslclient);
  }
  else {
    sld->preparePassiveTransfer(dst, dfile, true, ssl, !sourcesslclient);
  }


}

void TransferMonitor::engageDownload(
  const std::string & sfile, const std::shared_ptr<SiteLogic> & sls, FileList * fls,
  const std::shared_ptr<LocalFileList> & localfl, CommandOwner * co)
{
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
  this->srcco = co;
  if (!sls->lockDownloadConn(fls, &src, srcco, this)) return;
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
  ts = std::make_shared<TransferStatus>(TRANSFERSTATUS_TYPE_DOWNLOAD,
      sls->getSite()->getName(), "/\\", "", dfile, fls, spath,
      (FileList *)NULL, dpath, fls->getFile(sfile)->getSize(), 0, src, -1, ssl, clientactive);
  tm->addNewTransferStatus(ts);
  if (clientactive) {
    sls->preparePassiveTransfer(src, sfile, false, ssl);
  }
  else {
    lt = global->getLocalStorage()->activeModeDownload(this, dpath, dfile, ssl, sls->getConn(src));
    passiveReady(global->getLocalStorage()->getAddress(lt), lt->getPort());
  }
}

void TransferMonitor::engageUpload(
  const std::string & sfile, const std::shared_ptr<LocalFileList> & localfl,
  const std::shared_ptr<SiteLogic> & sld, FileList * fld, CommandOwner * co)
{
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
  this->dstco = co;
  std::unordered_map<std::string, LocalFile>::const_iterator it = localfl->find(sfile);
  if (it == localfl->end()) return;
  const LocalFile & lf = it->second;
  if (!sld->lockUploadConn(fld, &dst, dstco, this)) return;
  status = TM_STATUS_AWAITING_PASSIVE;
  int spol = sld->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  fld->touchFile(dfile, sld->getSite()->getUser(), true);
  clientactive = !sld->getSite()->hasBrokenPASV();
  ts = std::make_shared<TransferStatus>(TRANSFERSTATUS_TYPE_UPLOAD,
      "/\\", sld->getSite()->getName(), "", dfile, (FileList *)NULL, spath,
      fld, dpath, lf.getSize(), 0, 0, dst, ssl, clientactive);
  tm->addNewTransferStatus(ts);
  if (clientactive) {
    sld->preparePassiveTransfer(dst, dfile, false, ssl);
  }
  else {
    lt = global->getLocalStorage()->activeModeUpload(this, spath, sfile, ssl, sld->getConn(dst));
    passiveReady(global->getLocalStorage()->getAddress(lt), lt->getPort());
  }
}

void TransferMonitor::engageList(const std::shared_ptr<SiteLogic> & sls, int connid,
  bool hiddenfiles, FileList * fl, CommandOwner * co)
{
  reset();
  type = TM_TYPE_LIST;
  this->sls = sls;
  this->fls = fl;
  this->srcco = co;
  src = connid;
  this->hiddenfiles = hiddenfiles;
  ssl = sls->getSite()->getTLSMode() != TLSMode::NONE;
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
    ++ticker;
    if (type == TM_TYPE_FXP) {
      updateFXPSizeSpeed();
      if (ticker % 20 == 0) { // run once per second
        checkForDeadFXPTransfers();
      }
    }
    if (type == TM_TYPE_DOWNLOAD || type == TM_TYPE_UPLOAD) {
      if (ticker % 4 == 0) { // run every 200 ms
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
    sls->returnConn(src, type != TM_TYPE_LIST);
    return;
  }
  if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET) {
    targetError(TM_ERR_OTHER);
    sld->returnConn(dst, type != TM_TYPE_LIST);
    return;
  }
  status = TM_STATUS_AWAITING_ACTIVE;
  if (!!ts) {
    ts->setPassiveAddress(host + ":" + util::int2Str(port));
  }
  switch (type) {
    case TM_TYPE_FXP:
      if (fxpdstactive) {
        sld->prepareActiveTransfer(dst, dfile, true, host, port, ssl, !sourcesslclient);
      }
      else {
        sls->prepareActiveTransfer(src, sfile, true, host, port, ssl, sourcesslclient);
      }
      break;
    case TM_TYPE_DOWNLOAD:
      if (clientactive) {
        lt = global->getLocalStorage()->passiveModeDownload(this, dpath, dfile, host, port, ssl, sls->getConn(src));
      }
      else {
        sls->prepareActiveTransfer(src, sfile, false, host, port, ssl);
      }
      break;
    case TM_TYPE_UPLOAD:
      if (clientactive) {
        lt = global->getLocalStorage()->passiveModeUpload(this, spath, sfile, host, port, ssl, sld->getConn(dst));
      }
      else {
        sld->prepareActiveTransfer(dst, dfile, false, host, port, ssl);
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
    sls->returnConn(src, type != TM_TYPE_LIST);
    return;
  }
  if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET) {
    targetError(TM_ERR_OTHER);
    sld->returnConn(dst, type != TM_TYPE_LIST);
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

void TransferMonitor::sslDetails(const std::string & cipher, bool sessionreused) {
  if (!!ts) {
    ts->setCipher(cipher);
    ts->setSSLSessionReused(sessionreused);
  }
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
  else if (status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE) {
    transferFailed(ts, error);
  }
  else {
    finish();
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
  else if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET) {
    transferFailed(ts, error);
  }
  else {
    finish();
  }
}

void TransferMonitor::finish() {
  int span = timestamp - startstamp;
  if (span == 0) {
    span = 10;
  }
  if (type == TM_TYPE_FXP) {
    updateFXPSizeSpeed();
  }
  if (type == TM_TYPE_DOWNLOAD || type == TM_TYPE_UPLOAD) {
    updateLocalTransferSizeSpeed();
  }
  switch (type) {
    case TM_TYPE_FXP:
    case TM_TYPE_DOWNLOAD: {
      File * srcfile = fls->getFile(sfile);
      if (srcfile) {
        unsigned long long int size = std::max(srcfile->getSize(), ts->knownTargetSize());
        unsigned int speed = size / span;
        ts->setTargetSize(size);
        ts->setSpeed(speed);
        ts->setTimeSpent(span / 1000);
        if (type == TM_TYPE_FXP) {
          fld->setFileUpdateFlag(dfile, size, speed, sls->getSite(), sld->getSite(), srcco, dstco);
        }
        else {
          global->getStatistics()->addTransferStatsFile(STATS_DOWN, lt->size());
          sls->getSite()->addTransferStatsFile(STATS_DOWN, lt->size());
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
      sld->getSite()->addTransferStatsFile(STATS_UP, size);
      global->getStatistics()->addTransferStatsFile(STATS_UP, size);
      break;
    }
    case TM_TYPE_LIST:
      sls->listCompleted(src, storeid, fls, srcco);
      break;
  }
  if (!!ts) {
    ts->setFinished();
  }
  tm->transferSuccessful(ts);
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
    if (!!sld && !sld->getConn(dst)->isProcessing()) {
      sld->returnConn(dst, type != TM_TYPE_LIST);
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
  error = err;
  status = TM_STATUS_SOURCE_ERROR_AWAITING_TARGET;
}

void TransferMonitor::targetError(TransferError err) {
  util::assert(status == TM_STATUS_AWAITING_ACTIVE ||
               status == TM_STATUS_AWAITING_PASSIVE ||
               status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
               status == TM_STATUS_TRANSFERRING ||
               status == TM_STATUS_TRANSFERRING_SOURCE_COMPLETE);
  if (fld != NULL) {
    if  (err != TM_ERR_DUPE) {
      fld->removeFile(dfile);
    }
    else {
      fld->finishUpload(dfile);
    }
  }
  partialcompletestamp = timestamp;
  if (status == TM_STATUS_AWAITING_PASSIVE || status == TM_STATUS_AWAITING_ACTIVE) {
    if (!!sls && !sls->getConn(src)->isProcessing()) {
      sls->returnConn(src, type != TM_TYPE_LIST);
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
  error = err;
  status = TM_STATUS_TARGET_ERROR_AWAITING_SOURCE;
}

std::shared_ptr<TransferStatus> TransferMonitor::getTransferStatus() const {
  return ts;
}

void TransferMonitor::updateFXPSizeSpeed() {
  File * srcfile = fls->getFile(sfile);
  if (srcfile) {
    int srctouch = srcfile->getTouch();
    if (latestsrctouch != srctouch) {
      latestsrctouch = srctouch;
      unsigned long long int srcfilesize = srcfile->getSize();
      if (srcfilesize > ts->sourceSize()) {
        ts->setSourceSize(srcfilesize);
      }
    }
  }
  File * dstfile = fld->getFile(dfile);
  if (dstfile) {
    unsigned long long int dstfilesize = dstfile->getSize();
    int span = timestamp - startstamp;
    int dsttouch = dstfile->getTouch();
    if (!span) {
      span = 10;
    }

    // if the file list has been updated (as seen on the file's touch stamp
    // the speed shall be recalculated.
    if (latestdsttouch != dsttouch) {
      latestdsttouch = dsttouch;
      setTargetSizeSpeed(dstfilesize, span);
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
  sls.reset();
  sld.reset();
  fls = NULL;
  fld = NULL;
  lt = NULL;
  ts.reset();
  srcco = NULL;
  dstco = NULL;
  clientactive = true;
  fxpdstactive = true;
  ssl = false;
  timestamp = 0;
  startstamp = 0;
  partialcompletestamp = 0;
  rawbufqueue.clear();
}

bool TransferMonitor::willFail() const {
  return status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
         status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE;
}

void TransferMonitor::transferFailed(const std::shared_ptr<TransferStatus> & ts, TransferError err) {
  if (status == TM_STATUS_SOURCE_ERROR_AWAITING_TARGET ||
      status == TM_STATUS_TARGET_ERROR_AWAITING_SOURCE)
  {
    if (error == TM_ERR_DUPE) {
      err = TM_ERR_DUPE;
    }
  }
  if (!!ts) {
    if (err == TM_ERR_DUPE) {
      ts->setDupe();
    }
    else {
      ts->setFailed();
    }
  }
  if (type == TM_TYPE_FXP && err != TM_ERR_DUPE) {
    std::string extension = File::getExtension(dfile);
    if (extension != "sfv" && extension != "nfo") {
      sls->getSite()->pushTransferSpeed(sld->getSite()->getName(), 0, 0);
    }
  }
  tm->transferFailed(ts, err);
  status = TM_STATUS_IDLE;
}

void TransferMonitor::newRawBufferLine(const std::pair<std::string, std::string> & line) {
  if (!!ts) {
    if (!rawbufqueue.empty()) {
      for (std::list<std::pair<std::string, std::string> >::const_iterator it = rawbufqueue.begin(); it != rawbufqueue.end(); it++) {
        ts->addLogLine(it->first + " " + it->second);
      }
      rawbufqueue.clear();
    }
    ts->addLogLine(line.first + " " + line.second);
  }
  else {
    rawbufqueue.push_back(line);
  }
}
