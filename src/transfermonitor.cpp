#include "transfermonitor.h"

TransferMonitor::TransferMonitor() {
  status = 0;
  activedownload = false;
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
  if (!sld->getSite()->hasBrokenPASV()) {
    activedownload = true;
    sld->preparePassiveUpload(dst, this, fld, file, ssl);
  }
  else {
    sls->preparePassiveDownload(src, this, fls, file, ssl);
  }
}

void TransferMonitor::tick(int msg) {

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

}

void TransferMonitor::sourceComplete() {
  sourcecomplete = true;
  if (targetcomplete) {
    if (status != 0) {
      fld->touchFile(file, sld->getSite()->getUser());
      status = 0;
    }
  }
}

void TransferMonitor::targetComplete() {
  targetcomplete = true;
  if (sourcecomplete) {
    if (status != 0) {
      fld->touchFile(file, sld->getSite()->getUser());
      status = 0;
    }
  }
}

void TransferMonitor::sourceError(int err) {
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
  status = 0;
}

void TransferMonitor::targetError(int err) {
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
  status = 0;
}
