#include "transfer.h"

Transfer::Transfer(std::string file, SiteThread * sts, FileList * fls, SiteThread * std, FileList * fld) {
  this->sts = sts;
  this->std = std;
  this->fls = fls;
  this->fld = fld;
  this->file = file;
}

void Transfer::run() {
  if (sts->getSite()->hasBrokenPASV() && std->getSite()->hasBrokenPASV()) return;
  if (!sts->getDownloadThread(fls, file, &src)) return;
  if (!std->getUploadThread(fld, file, &dst)) {
    sts->returnThread(src);
    sts->transferComplete(true);
    return;
  }
  std::string * addr;
  if (!sts->getSite()->hasBrokenPASV()) {
    if (sts->getSite()->needsPRET()) {
      if (!src->doPRETRETR(file)) {
        cancelTransfer();
        return;
      }
    }
    if (fld->getFile(file) || !src->doPASV(&addr)) {
      cancelTransfer();
      return;
    }
    if (fld->getFile(file) || !dst->doPORT(*addr)) {
      cancelTransfer();
      return;
    }
    if (fld->getFile(file) || !dst->doSTOR(file)) {
      cancelTransfer();
      return;
    }
    if (!src->doRETR(file)) {
      src->doPASV(&addr);
      sts->returnThread(src);
      sts->transferComplete(true);
      dst->awaitTransferComplete();
      std->transferComplete(false);
      std->returnThread(dst);
      return;
    }
  }
  else {
    if (std->getSite()->needsPRET()) {
      if (!dst->doPRETSTOR(file)) {
        cancelTransfer();
        return;
      }
    }
    if (fld->getFile(file) || !dst->doPASV(&addr)) {
      cancelTransfer();
      return;
    }
    if (fld->getFile(file) || !src->doPORT(*addr)) {
      cancelTransfer();
      return;
    }
    if(fld->getFile(file) || !src->doRETR(file)) {
      cancelTransfer();
      return;
    }
    if (fld->getFile(file) || !dst->doSTOR(file)) {
      dst->doPASV(&addr);
      std->transferComplete(false);
      std->returnThread(dst);
      src->awaitTransferComplete();
      sts->transferComplete(true);
      sts->returnThread(src);
      return;
    }
  }
  int start = global->ctimeMSec();
  fld->touchFile(file, std->getSite()->getUser());
  src->awaitTransferComplete();
  sts->transferComplete(true);
  sts->returnThread(src);
  dst->awaitTransferComplete();
  std->transferComplete(false);
  std->returnThread(dst);
  int span = global->ctimeMSec() - start;
  int size = fls->getFile(file)->getSize();
  int speed = size / span;
  //std::cout << "[ " << sts->getSite()->getName() << " -> " << std->getSite()->getName() << " ] - " << file << " - " << speed << " kB/s" << std::endl;
  fld->setFileUpdateFlag(file, speed, sts->getSite(), std->getSite()->getName());
}

void Transfer::cancelTransfer() {
  sts->transferComplete(true);
  std->transferComplete(false);
  sts->returnThread(src);
  std->returnThread(dst);
}
