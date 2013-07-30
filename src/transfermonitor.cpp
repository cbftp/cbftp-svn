#include "transfermonitor.h"

TransferMonitor::TransferMonitor() {
  status = 0;
  activedownload = false;
}

bool TransferMonitor::idle() {
  return status == 0;
}

void TransferMonitor::engage(std::string file, SiteLogic * sts, FileList * fls, SiteLogic * std, FileList * fld) {
  this->sts = sts;
  this->std = std;
  this->fls = fls;
  this->fld = fld;
  this->file = file;
  activedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  ssl = false;
  if (!sts->lockDownloadConn(fls, file, &src)) return;
  if (!std->lockUploadConn(fld, file, &dst)) {
    sts->returnConn(src);
    return;
  }
  status = 1;
  if (sts->getSite()->SSLFXPForced() || std->getSite()->SSLFXPForced()) {
    ssl = true;
  }
  if (!sts->getSite()->hasBrokenPASV()) {
    activedownload = true;
    std->preparePassiveUpload(dst, this, fld, file, ssl);
  }
  else {
    sts->preparePassiveDownload(src, this, fls, file, ssl);
  }
}

void TransferMonitor::tick(int msg) {

}

void TransferMonitor::passiveReady(std::string addr) {
  if (activedownload) {
    sts->activeDownload(src, this, fls, file, addr, ssl);
    std->passiveUpload(dst);
  }
  else {
    std->activeUpload(dst, this, fld, file, addr, ssl);
    sts->passiveDownload(src);
  }
  fld->touchFile(file, std->getSite()->getUser());
}

void TransferMonitor::sourceComplete() {
  sourcecomplete = true;
  if (targetcomplete) {
    status = 0;
  }
}

void TransferMonitor::targetComplete() {
  targetcomplete = true;
  if (sourcecomplete) {
    status = 0;
  }
}

void TransferMonitor::sourceError(int err) {

}

void TransferMonitor::targetError(int err) {

}
