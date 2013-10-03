#include "transfermonitor.h"

#include "site.h"
#include "sitelogic.h"
#include "siterace.h"
#include "ftpconn.h"
#include "filelist.h"
#include "tickpoke.h"
#include "globalcontext.h"
#include "file.h"

extern GlobalContext * global;

TransferMonitor::TransferMonitor() {
  status = 0;
  activedownload = false;
  global->getTickPoke()->startPoke(this, 50, 0);
  timestamp = 0;
}

bool TransferMonitor::idle() {
  return status == 0;
}

void TransferMonitor::engage(std::string file, SiteLogic * sls, FileList * fls, SiteLogic * sld, FileList * fld) {
  this->sls = sls;
  this->sld = sld;
  this->fls = fls;
  this->fld = fld;
  this->file = file;
  activedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  timestamp = 0;
  ssl = false;
  if (!sls->lockDownloadConn(fls, file, &src)) return;
  if (!sld->lockUploadConn(fld, file, &dst)) {
    sls->returnConn(src);
    return;
  }
  status = 1;
  if (sls->getSite()->SSLFXPForced() || sld->getSite()->SSLFXPForced()) {
    ssl = true;
  }
  fld->touchFile(file, sld->getSite()->getUser(), true);
  fls->getFile(file)->download();
  if (!sld->getSite()->hasBrokenPASV()) {
    activedownload = true;
    sld->preparePassiveUpload(dst, this, fld, file, ssl);
  }
  else {
    sls->preparePassiveDownload(src, this, fls, file, ssl);
  }
}

void TransferMonitor::tick(int msg) {
  timestamp += 50;
}

void TransferMonitor::passiveReady(std::string addr) {
  if (activedownload) {
    sls->activeDownload(src, this, fls, file, addr, ssl);
    sld->passiveUpload(dst);
  }
  else {
    sld->activeUpload(dst, this, fld, file, addr, ssl);
    sls->passiveDownload(src);
  }
  startstamp = timestamp;
}

void TransferMonitor::sourceComplete() {
  sourcecomplete = true;
  File * fileobj = fls->getFile(file);
  if (fileobj != NULL) {
    fileobj->finishDownload();
  }
  if (targetcomplete) {
    finish();
  }
}

void TransferMonitor::targetComplete() {
  targetcomplete = true;
  File * fileobj = fld->getFile(file);
  if (fileobj != NULL) {
    fileobj->finishUpload();
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
    File * srcfile = fls->getFile(file);
    if (srcfile) {
      long int size = srcfile->getSize();
      int speed = size / span;
      //std::cout << "[ " << sls->getSite()->getName() << " -> " << sld->getSite()->getName() << " ] - " << file << " - " << speed << " kB/s" << std::endl;
      if (size > 1000000) {
        fld->setFileUpdateFlag(file, size, speed, sls->getSite(), sld->getSite()->getName());
      }
    }
  }
  status = 0;
}

void TransferMonitor::sourceError(int err) {
  File * fileobj = fls->getFile(file);
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
      fls->downloadAttemptFail(file);
      break;
  }
  sourcecomplete = true;
  status = 13;
  if (targetcomplete) {
    status = 0;
  }
}

void TransferMonitor::targetError(int err) {
  File * fileobj = fld->getFile(file);
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
      fld->uploadAttemptFail(file);
      break;
  }
  targetcomplete = true;
  status = 13;
  if (sourcecomplete) {
    status = 0;
  }
}
