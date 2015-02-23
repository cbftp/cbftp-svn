#include "transfermonitor.h"

#include "site.h"
#include "sitelogic.h"
#include "siterace.h"
#include "ftpconn.h"
#include "filelist.h"
#include "tickpoke.h"
#include "globalcontext.h"
#include "file.h"
#include "localstorage.h"
#include "transfermanager.h"
#include "transferstatus.h"
#include "localtransfer.h"

extern GlobalContext * global;

TransferMonitor::TransferMonitor(TransferManager * tm) {
  this->tm = tm;
  status = TM_STATUS_IDLE;
  activedownload = false;
  timestamp = 0;
  localtransferspeedticker = 0;
  global->getTickPoke()->startPoke(this, "TransferMonitor", TICKINTERVAL, 0);
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
  if (!sls->lockDownloadConn(spath, &src)) {
    tm->transferFailed(ts, 4);
    return;
  }
  if (!sld->lockUploadConn(dpath, &dst)) {
    sls->returnConn(src);
    tm->transferFailed(ts, 5);
    return;
  }
  status = TM_STATUS_AWAITING_PASSIVE;
  ts = Pointer<TransferStatus>(new TransferStatus(TRANSFERSTATUS_TYPE_FXP, sls->getSite()->getName(),
      sld->getSite()->getName(), "", dfile, fls->getPath(), fld->getPath(),
      fls->getFile(sfile)->getSize(),
      sls->getSite()->getAverageSpeed(sld->getSite()->getName())));
  tm->addNewTransferStatus(ts);
  int spol = sls->getSite()->getSSLTransferPolicy();
  int dpol = sld->getSite()->getSSLTransferPolicy();
  if (spol != SITE_SSL_ALWAYS_OFF && dpol != SITE_SSL_ALWAYS_OFF &&
      (spol == SITE_SSL_ALWAYS_ON || dpol == SITE_SSL_ALWAYS_ON ||
      (spol == SITE_SSL_PREFER_ON && dpol == SITE_SSL_PREFER_ON))) {
    ssl = true;
  }
  fld->touchFile(dfile, sld->getSite()->getUser(), true);
  latesttouch = fld->getFile(dfile)->getTouch();
  fls->getFile(sfile)->download();
  if (!sld->getSite()->hasBrokenPASV()) {
    activedownload = true;
    sld->preparePassiveUpload(dst, this, dpath, dfile, true, ssl);
  }
  else {
    sls->preparePassiveDownload(src, this, spath, sfile, true, ssl);
  }
}

void TransferMonitor::engageDownload(std::string sfile, SiteLogic * sls, FileList * fls, std::string dpath) {
  reset();
  type = TM_TYPE_LOCAL;
  this->sls = sls;
  this->sfile = sfile;
  this->dfile = sfile;
  this->spath = fls->getPath();
  this->dpath = dpath;
  this->fls = fls;
  if (!sls->lockDownloadConn(spath, &src)) return;
  status = TM_STATUS_AWAITING_PASSIVE;
  ts = Pointer<TransferStatus>(new TransferStatus(TRANSFERSTATUS_TYPE_DOWNLOAD,
      sls->getSite()->getName(), "/\\", "", dfile, spath,
      dpath, fls->getFile(sfile)->getSize(), 0));
  tm->addNewTransferStatus(ts);
  int spol = sls->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  if (!global->getLocalStorage()->directoryExistsWritable(dpath)) {
    global->getLocalStorage()->createDirectoryRecursive(dpath);
  }
  if (!sls->getSite()->hasBrokenPASV()) {
    sls->preparePassiveDownload(src, this, spath, sfile, false, ssl);
  }
  else {
    activedownload = true;
    // client active mode is not implemented yet
  }
}

void TransferMonitor::engageUpload(std::string sfile, std::string spath, SiteLogic * sld, FileList * fld) {
  reset();
  type = TM_TYPE_LOCAL;
  this->sfile = sfile;
  this->dfile = sfile;
  this->spath = spath;
  this->sld = sld;
  this->fld = fld;
  this->dpath = fld->getPath();
  if (!sls->lockUploadConn(dpath, &dst)) return;
  status = TM_STATUS_AWAITING_PASSIVE;
  ts = Pointer<TransferStatus>(new TransferStatus(TRANSFERSTATUS_TYPE_UPLOAD,
      "/\\", sld->getSite()->getName(), "", dfile, spath,
      dpath, fls->getFile(sfile)->getSize(), 0));
  tm->addNewTransferStatus(ts);
  int spol = sld->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  if (!sls->getSite()->hasBrokenPASV()) {
    sls->preparePassiveUpload(dst, this, dpath, dfile, false, ssl);
  }
  else {
    activedownload = true;
    // client active mode is not implemented yet
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
    activedownload = true;
    // client active mode is not implemented yet
  }
}

void TransferMonitor::tick(int msg) {
  if (status != TM_STATUS_IDLE) {
    timestamp += TICKINTERVAL;
    if (type == TM_TYPE_FXP) {
      File * file = fld->getFile(dfile);
      if (file) {
        unsigned int filesize = file->getSize();
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
    if (type == TM_TYPE_LOCAL) {
      if (localtransferspeedticker++ % 4 == 0 && lt) { // run every 200 ms
        unsigned int filesize = lt->size();
        int span = timestamp - startstamp;
        if (!span) {
          span = 10;
        }
        setTargetSizeSpeed(filesize, span);
        ts->setTimeSpent(span / 1000);
      }
    }
  }
}

void TransferMonitor::passiveReady(std::string addr) {
  if (status != TM_STATUS_AWAITING_PASSIVE) {
    *(int*)0=0; // crash on purpose
  }
  status = TM_STATUS_AWAITING_ACTIVE;
  switch (type) {
    case TM_TYPE_FXP:
      if (activedownload) {
        sls->prepareActiveDownload(src, this, spath, sfile, addr, ssl);
      }
      else {
        sld->prepareActiveUpload(dst, this, dpath, dfile, addr, ssl);
      }
      break;
    case TM_TYPE_LOCAL:
      if (activedownload) {
        // client active mode is not implemented yet
      }
      else {
        lt = global->getLocalStorage()->passiveDownload(this, dpath, dfile, addr, ssl, sls->getConn(src));
      }
      break;
    case TM_TYPE_LIST:
      if (activedownload) {
        // client active mode is not implemented yet
      }
      else {
        lt = global->getLocalStorage()->passiveDownload(this, addr, ssl, sls->getConn(src));
        storeid = lt->getStoreId();
      }
      break;
  }
}

void TransferMonitor::activeReady() {
  if (status != TM_STATUS_AWAITING_ACTIVE) {
    *(int*)0=0; // crash on purpose
  }
  status = TM_STATUS_TRANSFERRING;
  switch (type) {
    case TM_TYPE_FXP:
      sls->download(src);
      sld->upload(dst);
      break;
    case TM_TYPE_LOCAL:
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
  }
  startstamp = timestamp;
}

void TransferMonitor::sourceComplete() {
  if (status != TM_STATUS_ERROR_AWAITING_PEER && status != TM_STATUS_TRANSFERRING) {
    *(int*)0=0; // crash on purpose
  }
  sourcecomplete = true;
  if (fls != NULL) {
    File * fileobj = fls->getFile(sfile);
    if (fileobj != NULL) {
      fileobj->finishDownload();
    }
  }
  if (targetcomplete) {
    finish();
  }
}

void TransferMonitor::targetComplete() {
  if (status != TM_STATUS_ERROR_AWAITING_PEER && status != TM_STATUS_TRANSFERRING) {
    *(int*)0=0; // crash on purpose
  }
  targetcomplete = true;
  if (fld != NULL) {
    File * fileobj = fld->getFile(dfile);
    if (fileobj != NULL) {
      fileobj->finishUpload();
    }
  }
  if (sourcecomplete) {
    finish();
  }
}

void TransferMonitor::finish() {
  if (status != TM_STATUS_IDLE && status != TM_STATUS_ERROR_AWAITING_PEER) {
    int span = timestamp - startstamp;
    if (span == 0) {
      span = 10;
    }
    switch (type) {
      case TM_TYPE_FXP:
      case TM_TYPE_LOCAL: {
        File * srcfile = fls->getFile(sfile);
        if (srcfile) {
          long int size = srcfile->getSize();
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
      case TM_TYPE_LIST:
        sls->listCompleted(src, storeid);
        break;
    }
  }
  if (!!ts) {
    ts->setFinished();
  }
  if (status != TM_STATUS_ERROR_AWAITING_PEER) {
    tm->transferSuccessful(ts);
  }
  else {
    tm->transferFailed(ts, 6);
    if (type == TM_TYPE_FXP) {
      sls->getSite()->pushTransferSpeed(sld->getSite()->getName(), 0);
    }
  }
  status = TM_STATUS_IDLE;
}

void TransferMonitor::sourceError(int err) {
  if (status != TM_STATUS_AWAITING_ACTIVE && status != TM_STATUS_AWAITING_PASSIVE &&
      status != TM_STATUS_ERROR_AWAITING_PEER && status != TM_STATUS_TRANSFERRING) {
    *(int*)0=0; // crash on purpose
  }
  if (fls != NULL) {
    File * fileobj = fls->getFile(sfile);
    if (fileobj != NULL) {
      fileobj->finishDownload();
    }
    switch (err) {
      case 0: // PRET RETR failed
        fls->downloadFail(sfile);
        break;
      case 1: // RETR failed
        fls->downloadFail(sfile);
        break;
      case 2: // RETR post failed
      case 3: // other failure
        fls->downloadAttemptFail(sfile);
        break;
    }
  }
  sourcecomplete = true;
  if (status == TM_STATUS_AWAITING_PASSIVE || status == TM_STATUS_AWAITING_ACTIVE) {
    if (sld != NULL) {
      sld->returnConn(dst);
      File * fileobj = fld->getFile(dfile);
      if (fileobj != NULL) {
        fileobj->finishUpload();
      }
      tm->transferFailed(ts, err);
      status = TM_STATUS_IDLE;
      return;
    }
  }
  if (targetcomplete || (type == TM_TYPE_LOCAL && lt == NULL)) {
    if (type == TM_TYPE_FXP) {
      sls->getSite()->pushTransferSpeed(sld->getSite()->getName(), 0);
    }
    tm->transferFailed(ts, err);
    status = TM_STATUS_IDLE;
    return;
  }
  status = TM_STATUS_ERROR_AWAITING_PEER;
}

void TransferMonitor::targetError(int err) {
  if (status != TM_STATUS_AWAITING_ACTIVE && status != TM_STATUS_AWAITING_PASSIVE &&
      status != TM_STATUS_ERROR_AWAITING_PEER && status != TM_STATUS_TRANSFERRING) {
    *(int*)0=0; // crash on purpose
  }
  if (fld != NULL) {
    File * fileobj = fld->getFile(dfile);
    if (fileobj != NULL) {
      fileobj->finishUpload();
    }
    switch (err) {
      case 0: // PRET STOR failed
        fld->uploadFail(dfile);
        break;
      case 1: // STOR failed
        fld->uploadFail(dfile);
        break;
      case 2: // STOR post failed
      case 3: // other failure
        fld->uploadAttemptFail(dfile);
        break;
    }
  }
  targetcomplete = true;
  if (status == TM_STATUS_AWAITING_PASSIVE || status == TM_STATUS_AWAITING_ACTIVE) {
    if (sls != NULL) {
      sls->returnConn(src);
      if (fls != NULL) { // NULL in case of LIST
        File * fileobj = fls->getFile(sfile);
        if (fileobj != NULL) {
          fileobj->finishDownload();
        }
      }
      tm->transferFailed(ts, err);
      status = TM_STATUS_IDLE;
      return;
    }
  }
  if (sourcecomplete || (type == TM_TYPE_LOCAL && lt == NULL)) {
    if (type == TM_TYPE_FXP) {
      sls->getSite()->pushTransferSpeed(sld->getSite()->getName(), 0);
    }
    tm->transferFailed(ts, err);
    status = TM_STATUS_IDLE;
    return;
  }
  status = TM_STATUS_ERROR_AWAITING_PEER;
}

Pointer<TransferStatus> TransferMonitor::getTransferStatus() const {
  return ts;
}

void TransferMonitor::setTargetSizeSpeed(unsigned int filesize, int span) {
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

int TransferMonitor::getStatus() const {
  return status;
}

void TransferMonitor::reset() {
  sls = NULL;
  sld = NULL;
  fls = NULL;
  fld = NULL;
  lt = NULL;
  ts.reset();
  activedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  ssl = false;
  timestamp = 0;
}
