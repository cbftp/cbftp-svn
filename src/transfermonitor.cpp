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

extern GlobalContext * global;

TransferMonitor::TransferMonitor(TransferManager * tm) {
  this->tm = tm;
  status = 0;
  activedownload = false;
  timestamp = 0;
  global->getTickPoke()->startPoke(this, "TransferMonitor", 50, 0);
}

bool TransferMonitor::idle() {
  return status == 0;
}

void TransferMonitor::engage(std::string file, SiteLogic * sls, FileList * fls, SiteLogic * sld, FileList * fld) {
  this->sls = sls;
  this->sld = sld;
  this->fls = fls;
  this->spath = fls->getPath();
  this->fld = fld;
  this->dpath = fld->getPath();
  this->file = file;
  activedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  passiveready = false;
  type = TM_TYPE_FXP;
  timestamp = 0;
  ssl = false;
  if (!sls->lockDownloadConn(spath, file, &src)) {
    tm->transferFailed(this, 4);
  }
  if (!sld->lockUploadConn(dpath, file, &dst)) {
    sls->returnConn(src);
    tm->transferFailed(this, 5);
    return;
  }
  status = 1;
  int spol = sls->getSite()->getSSLTransferPolicy();
  int dpol = sld->getSite()->getSSLTransferPolicy();
  if (spol != SITE_SSL_ALWAYS_OFF && dpol != SITE_SSL_ALWAYS_OFF &&
      (spol == SITE_SSL_ALWAYS_ON || dpol == SITE_SSL_ALWAYS_ON ||
      (spol == SITE_SSL_PREFER_ON && dpol == SITE_SSL_PREFER_ON))) {
    ssl = true;
  }
  fld->touchFile(file, sld->getSite()->getUser(), true);
  fls->getFile(file)->download();
  if (!sld->getSite()->hasBrokenPASV()) {
    activedownload = true;
    sld->preparePassiveUpload(dst, this, dpath, file, true, ssl);
  }
  else {
    sls->preparePassiveDownload(src, this, spath, file, true, ssl);
  }
}

void TransferMonitor::engage(std::string file, SiteLogic * sls, FileList * fls) {
  this->sls = sls;
  this->file = file;
  this->spath = fls->getPath();
  this->fls = fls;
  fld = NULL;
  activedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  passiveready = false;
  type = TM_TYPE_LOCAL;
  timestamp = 0;
  ssl = false;
  if (!sls->lockDownloadConn(spath, file, &src)) return;
  status = 1;
  int spol = sls->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  if (!sls->getSite()->hasBrokenPASV()) {
    sls->preparePassiveDownload(src, this, spath, file, false, ssl);
  }
  else {
    activedownload = true;
    // ?
  }
}

void TransferMonitor::engage(SiteLogic * sls, int connid) {
  fls = NULL;
  fld = NULL;
  src = connid;
  activedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  passiveready = false;
  type = TM_TYPE_LIST;
  timestamp = 0;
  this->sls = sls;
  ssl = false;
  int spol = sls->getSite()->getSSLTransferPolicy();
  if (spol == SITE_SSL_ALWAYS_ON || spol == SITE_SSL_PREFER_ON) {
    ssl = true;
  }
  status = 1;
  if (!sls->getSite()->hasBrokenPASV()) {
    sls->preparePassiveList(src, this, ssl);
  }
  else {
    activedownload = true;
    // ?
  }
}

void TransferMonitor::tick(int msg) {
  timestamp += 50;
}

void TransferMonitor::passiveReady(std::string addr) {
  passiveready = true;
  switch (type) {
    case TM_TYPE_FXP:
      if (activedownload) {
        sls->prepareActiveDownload(src, this, spath, file, addr, ssl);
      }
      else {
        sld->prepareActiveUpload(dst, this, dpath, file, addr, ssl);
      }
      break;
    case TM_TYPE_LOCAL:
      if (activedownload) {
        // ?
      }
      else {
        global->getLocalStorage()->passiveDownload(this, file, addr, ssl, sls->getConn(src));
      }
      break;
    case TM_TYPE_LIST:
      if (activedownload) {
        // ?
      }
      else {
        storeid = global->getLocalStorage()->passiveDownload(this, addr, ssl, sls->getConn(src));
      }
      break;
  }
}

void TransferMonitor::activeReady() {
  switch (type) {
    case TM_TYPE_FXP:
      sls->download(src);
      sld->upload(dst);
      break;
    case TM_TYPE_LOCAL:
      sls->download(src);
      break;
    case TM_TYPE_LIST:
      sls->list(src);
      break;
  }
  startstamp = timestamp;
}

void TransferMonitor::sourceComplete() {
  sourcecomplete = true;
  if (fls != NULL) {
    File * fileobj = fls->getFile(file);
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
    File * fileobj = fld->getFile(file);
    if (fileobj != NULL) {
      fileobj->finishUpload();
    }
  }
  if (sourcecomplete) {
    finish();
  }
}

void TransferMonitor::finish() {
  if (status != 0 && status != 13) {
    int span = timestamp - startstamp;
    if (span == 0) {
      span = 10;
    }
    switch (type) {
      case TM_TYPE_FXP: {
        File * srcfile = fls->getFile(file);
        if (srcfile) {
          long int size = srcfile->getSize();
          int speed = size / span;
          //std::cout << "[ " << sls->getSite()->getName() << " -> " << sld->getSite()->getName() << " ] - " << file << " - " << speed << " kB/s" << std::endl;
          if (size > 1000000) {
            fld->setFileUpdateFlag(file, size, speed, sls->getSite(), sld->getSite()->getName());
          }
        }
        break;
      }
      case TM_TYPE_LIST:
        sls->listCompleted(src, storeid);
        break;
    }
  }
  tm->transferSuccessful(this);
  status = 0;
}

void TransferMonitor::sourceError(int err) {
  File * fileobj;
  if (type != TM_TYPE_LIST) {
    fileobj = fls->getFile(file);
    if (fileobj != NULL) {
      fileobj->finishDownload();
    }
    switch (err) {
      case 0: // PRET RETR failed
        fls->downloadFail(file);
        break;
      case 1: // RETR failed
        fls->downloadFail(file);
        break;
      case 2: // RETR post failed
      case 3: // other failure
        fls->downloadAttemptFail(file);
        break;
    }
    sourcecomplete = true;
    status = 13;
  }
  if (!passiveready) {
    if (type == TM_TYPE_FXP) {
      sld->returnConn(dst);
      fileobj = fld->getFile(file);
      if (fileobj != NULL) {
        fileobj->finishUpload();
      }
    }
    tm->transferFailed(this, err);
    status = 0;
  }
  else if (sourcecomplete) {
    tm->transferFailed(this, err);
    status = 0;
  }
}

void TransferMonitor::targetError(int err) {
  File * fileobj;
  if (type != TM_TYPE_LIST) {
    fileobj = fld->getFile(file);
    if (fileobj != NULL) {
      fileobj->finishUpload();
    }
    switch (err) {
      case 0: // PRET STOR failed
        fld->uploadFail(file);
        break;
      case 1: // STOR failed
        fld->uploadFail(file);
        break;
      case 2: // STOR post failed
      case 3: // other failure
        fld->uploadAttemptFail(file);
        break;
    }
  }
  targetcomplete = true;
  status = 13;
  if (!passiveready) {
    sls->returnConn(src);
    fileobj = fls->getFile(file);
    if (fileobj != NULL) {
      fileobj->finishDownload();
    }
    tm->transferFailed(this, err);
    status = 0;
  }
  else if (sourcecomplete) {
    tm->transferFailed(this, err);
    status = 0;
  }
}
