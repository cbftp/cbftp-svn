#include "transfermanager.h"

#include "scoreboardelement.h"
#include "transfermonitor.h"
#include "uibase.h"
#include "globalcontext.h"
#include "transferstatus.h"
#include "localfilelist.h"
#include "transferstatuscallback.h"

extern GlobalContext * global;

TransferManager::TransferManager() {
}

TransferManager::~TransferManager() {

}

void TransferManager::getFileList(SiteLogic * sl, int connid, bool hiddenfiles) {
  Pointer<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageList(sl, connid, hiddenfiles);
}

Pointer<TransferStatus> TransferManager::suggestTransfer(std::string name, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld) {
  return suggestTransfer(name, src, fls, name, dst, fld);
}

Pointer<TransferStatus> TransferManager::suggestTransfer(std::string srcname, SiteLogic * src, FileList * fls, std::string dstname, SiteLogic * dst, FileList * fld) {
  Pointer<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageFXP(srcname, src, fls, dstname, dst, fld);
  return target->getTransferStatus();
}

Pointer<TransferStatus> TransferManager::suggestDownload(std::string name, SiteLogic * sl, FileList * filelist, Pointer<LocalFileList> & path) {
  Pointer<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageDownload(name, sl, filelist, path);
  return target->getTransferStatus();
}

Pointer<TransferStatus> TransferManager::suggestUpload(std::string name, Pointer<LocalFileList> & path, SiteLogic * sl, FileList * filelist) {
  Pointer<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageUpload(name, path, sl, filelist);
  return target->getTransferStatus();
}

Pointer<TransferMonitor> TransferManager::getAvailableTransferMonitor() {
  Pointer<TransferMonitor> target;
  std::list<Pointer<TransferMonitor> >::iterator it;
  for (it = transfermonitors.begin(); it != transfermonitors.end(); it++) {
    if ((*it)->idle()) {
      target = *it;
      break;
    }
  }
  if (!target) {
    target = makePointer<TransferMonitor>(this);
    transfermonitors.push_back(target);
  }
  return target;
}

void TransferManager::transferSuccessful(Pointer<TransferStatus> & ts) {
  if (!!ts) {
    if (ts->isAwaited()) {
      global->getUIBase()->backendPush();
    }
    TransferStatusCallback * callback = ts->getCallback();
    if (callback != NULL) {
      callback->transferSuccessful(ts);
    }
    moveTransferStatusToFinished(ts);
  }
}

void TransferManager::transferFailed(Pointer<TransferStatus> & ts, int err) {
  if (!!ts) {
    if (ts->isAwaited()) {
      global->getUIBase()->backendPush();
    }
    TransferStatusCallback * callback = ts->getCallback();
    if (callback != NULL) {
      callback->transferFailed(ts, err);
    }
    moveTransferStatusToFinished(ts);
  }
}

std::list<Pointer<TransferStatus> >::const_iterator TransferManager::ongoingTransfersBegin() const {
  return ongoingtransfers.begin();
}

std::list<Pointer<TransferStatus> >::const_iterator TransferManager::ongoingTransfersEnd() const {
  return ongoingtransfers.end();
}

std::list<Pointer<TransferStatus> >::const_iterator TransferManager::finishedTransfersBegin() const {
  return finishedtransfers.begin();
}

std::list<Pointer<TransferStatus> >::const_iterator TransferManager::finishedTransfersEnd() const {
  return finishedtransfers.end();
}

void TransferManager::addNewTransferStatus(Pointer<TransferStatus> & ts) {
  ongoingtransfers.push_front(ts);
}

void TransferManager::moveTransferStatusToFinished(Pointer<TransferStatus> & movets) {
  for (std::list<Pointer<TransferStatus> >::iterator it = ongoingtransfers.begin(); it != ongoingtransfers.end(); it++) {
    if (*it == movets) {
      ongoingtransfers.erase(it);
      break;
    }
  }
  if (finishedtransfers.size() > MAX_TRANSFER_HISTORY) {
    finishedtransfers.pop_back();
  }
  finishedtransfers.push_front(movets);
}
