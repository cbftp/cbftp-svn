#include "transfermonitor.h"

TransferMonitor::TransferMonitor() {
  status = 0;
  passivedownload = false;
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
  passivedownload = false;
  sourcecomplete = false;
  targetcomplete = false;
  if (!sts->lockDownloadConn(fls, file, &src)) return;
  if (!std->lockUploadConn(fld, file, &dst)) {
    sts->returnConn(src);
    return;
  }
  status = 1;
  if (!sts->getSite()->hasBrokenPASV()) {
    passivedownload = true;
    sts->preparePassiveDownload(src, this, fls, file);
  }
  else {
    std->preparePassiveUpload(dst, this, fld, file);
  }
}

void TransferMonitor::tick(int msg) {

}

void TransferMonitor::passiveReady(std::string addr) {
  if (passivedownload) {
    std->activeUpload(dst, this, fld, file, addr);
    sts->passiveDownload(src);
  }
  else {
    sts->activeDownload(src, this, fls, file, addr);
    std->passiveUpload(dst);
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
