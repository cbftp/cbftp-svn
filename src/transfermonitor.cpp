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
  this->sls = sls;
  this->sld = sld;
  this->fls = fls;
  this->spath = fls->getPath();
  this->fld = fld;
  this->dpath = fld->getPath();
  this->sfile = sfile;
  this->dfile = dfile;
  activedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  type = TM_TYPE_FXP;
  timestamp = 0;
  ssl = false;
  ts = NULL;
  if (!sls->lockDownloadConn(spath, sfile, &src)) {
    tm->transferFailed(this, 4);
    return;
  }
  if (!sld->lockUploadConn(dpath, dfile, &dst)) {
    sls->returnConn(src);
    tm->transferFailed(this, 5);
    return;
  }
  status = TM_STATUS_AWAITING_PASSIVE;
  ts = new TransferStatus(TRANSFERSTATUS_TYPE_FXP, sls->getSite()->getName(),
      sld->getSite()->getName(), "", dfile, fls->getPath(), fld->getPath(),
      fls->getFile(sfile)->getSize(),
      sls->getSite()->getAverageSpeed(sld->getSite()->getName()));
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

void TransferMonitor::engageDownload(std::string sfile, SiteLogic * sls, FileList * fls) {
  this->sls = sls;
  this->sfile = sfile;
  this->dfile = sfile;
  this->spath = fls->getPath();
  this->fls = fls;
  fld = NULL;
  lt = NULL;
  activedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  type = TM_TYPE_LOCAL;
  timestamp = 0;
  ssl = false;
  ts = NULL;
  if (!sls->lockDownloadConn(spath, sfile, &src)) return;
  status = TM_STATUS_AWAITING_PASSIVE;
  ts = new TransferStatus(TRANSFERSTATUS_TYPE_DOWNLOAD,
      sls->getSite()->getName(), "local", "", dfile, fls->getPath(),
      global->getLocalStorage()->getTempPath(), fls->getFile(sfile)->getSize(),
      0);
  tm->addNewTransferStatus(ts);
  int spol = sls->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  if (!sls->getSite()->hasBrokenPASV()) {
    sls->preparePassiveDownload(src, this, spath, sfile, false, ssl);
  }
  else {
    activedownload = true;
    // client active mode is not implemented yet
  }
}

void TransferMonitor::engageList(SiteLogic * sls, int connid, bool hiddenfiles) {
  fls = NULL;
  fld = NULL;
  src = connid;
  activedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  this->hiddenfiles = hiddenfiles;
  type = TM_TYPE_LIST;
  timestamp = 0;
  this->sls = sls;
  ssl = false;
  ts = NULL;
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
          setTargetSizeSpeed(ts, filesize, span);
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
        setTargetSizeSpeed(ts, filesize, span);
        ts->setTimeSpent(span / 1000);
      }
    }
  }
}

void TransferMonitor::passiveReady(std::string addr) {
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
        lt = global->getLocalStorage()->passiveDownload(this, dfile, addr, ssl, sls->getConn(src));
      }
      break;
    case TM_TYPE_LIST:
      if (activedownload) {
        // client active mode is not implemented yet
      }
      else {
        storeid = global->getLocalStorage()->passiveDownload(this, addr, ssl, sls->getConn(src));
      }
      break;
  }
}

void TransferMonitor::activeReady() {
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
  if (type == TM_TYPE_FXP) {
    ts->setFinished();
  }
  if (status != TM_STATUS_ERROR_AWAITING_PEER) {
    tm->transferSuccessful(this);
  }
  else {
    tm->transferFailed(this, 6);
    if (type == TM_TYPE_FXP) {
      sls->getSite()->pushTransferSpeed(sld->getSite()->getName(), 0);
    }
  }
  status = TM_STATUS_IDLE;
}

void TransferMonitor::sourceError(int err) {
  if (type == TM_TYPE_LIST) {
    tm->transferFailed(this, err);
    status = TM_STATUS_IDLE;
    return;
  }
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
  sourcecomplete = true;
  if (status == TM_STATUS_AWAITING_PASSIVE || status == TM_STATUS_AWAITING_ACTIVE) {
    if (type == TM_TYPE_FXP) {
      sld->returnConn(dst);
      fileobj = fld->getFile(dfile);
      if (fileobj != NULL) {
        fileobj->finishUpload();
      }
    }
    tm->transferFailed(this, err);
    status = TM_STATUS_IDLE;
    return;
  }
  if (type == TM_TYPE_LOCAL || targetcomplete) {
    if (type == TM_TYPE_FXP) {
      sls->getSite()->pushTransferSpeed(sld->getSite()->getName(), 0);
    }
    tm->transferFailed(this, err);
    status = TM_STATUS_IDLE;
    return;
  }
  status = TM_STATUS_ERROR_AWAITING_PEER;
}

void TransferMonitor::targetError(int err) {
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
  targetcomplete = true;
  if (status == TM_STATUS_AWAITING_PASSIVE || status == TM_STATUS_AWAITING_ACTIVE) {
    sls->returnConn(src);
    fileobj = fls->getFile(sfile);
    if (fileobj != NULL) {
      fileobj->finishDownload();
    }
    tm->transferFailed(this, err);
    status = TM_STATUS_IDLE;
    return;
  }
  if (sourcecomplete) {
    if (type == TM_TYPE_FXP) {
      sls->getSite()->pushTransferSpeed(sld->getSite()->getName(), 0);
    }
    tm->transferFailed(this, err);
    status = TM_STATUS_IDLE;
    return;
  }
  status = TM_STATUS_ERROR_AWAITING_PEER;
}

TransferStatus * TransferMonitor::getTransferStatus() const {
  return ts;
}

void TransferMonitor::setTargetSizeSpeed(TransferStatus * ts, unsigned int filesize, int span) {
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
