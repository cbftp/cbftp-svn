#include "transfer.h"

Transfer::Transfer(std::string file, SiteThread * sts, SiteRace * srs, SiteThread * std, SiteRace * srd) {
  this->sts = sts;
  this->std = std;
  this->srs = srs;
  this->srd = srd;
  this->file = file;
}

void Transfer::run() {
  if (sts->getSite()->hasBrokenPASV() && std->getSite()->hasBrokenPASV()) return;
  if (!sts->getDownloadThread(srs, file, &src)) return;
  if (!std->getUploadThread(srd, file, &dst)) {
    src->setReady();
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
    if (srd->getFileList()->getFile(file) || !src->doPASV(&addr)) {
      cancelTransfer();
      return;
    }
    if (srd->getFileList()->getFile(file) || !dst->doPORT(*addr)) {
      cancelTransfer();
      return;
    }
    if (srd->getFileList()->getFile(file) || !dst->doSTOR(file)) {
      cancelTransfer();
      return;
    }
    if (!src->doRETR(file)) {
      src->doPASV(&addr);
      src->setReady();
      sts->transferComplete(true);
      dst->awaitTransferComplete();
      std->transferComplete(false);
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
    if (srd->getFileList()->getFile(file) || !dst->doPASV(&addr)) {
      cancelTransfer();
      return;
    }
    if (srd->getFileList()->getFile(file) || !src->doPORT(*addr)) {
      cancelTransfer();
      return;
    }
    if(srd->getFileList()->getFile(file) || !src->doRETR(file)) {
      cancelTransfer();
      return;
    }
    if (srd->getFileList()->getFile(file) || !dst->doSTOR(file)) {
      dst->doPASV(&addr);
      dst->setReady();
      std->transferComplete(false);
      src->awaitTransferComplete();
      sts->transferComplete(true);
      return;
    }
  }
  int start = global->ctimeMSec();
  srd->getFileList()->touchFile(file, std->getSite()->getUser());
  src->awaitTransferComplete();
  sts->transferComplete(true);
  dst->awaitTransferComplete();
  std->transferComplete(false);
  int span = global->ctimeMSec() - start;
  int size = srs->getFileList()->getFile(file)->getSize();
  int speed = size / span;
  std::cout << "[ " << sts->getSite()->getName() << " -> " << std->getSite()->getName() << " ] - " << file << " - " << speed << " kB/s" << std::endl;
  srd->getFileList()->setFileUpdateFlag(file, speed, sts->getSite(), std->getSite()->getName());
}

void Transfer::cancelTransfer() {
  src->setReady();
  sts->transferComplete(true);
  dst->setReady();
  std->transferComplete(false);
}
