#include "transfermanager.h"

#include "scoreboardelement.h"
#include "transfermonitor.h"
#include "uibase.h"
#include "globalcontext.h"
#include "transferstatus.h"

extern GlobalContext * global;

TransferManager::TransferManager() {
  requestids = 0;
}

int TransferManager::download(std::string name, SiteLogic * sl, FileList * filelist, std::string path) {
  int id = requestids++;
  Pointer<TransferMonitor> target = getAvailableTransferMonitor();
  transferstatus[id] = TRANSFER_IN_PROGRESS_UI;
  transfermap[target.get()] = id;
  target->engageDownload(name, sl, filelist, path);
  return id;
}

int TransferManager::getFileList(SiteLogic * sl, int connid, bool hiddenfiles) {
  int id = requestids++;
  Pointer<TransferMonitor> target = getAvailableTransferMonitor();
  transferstatus[id] = TRANSFER_IN_PROGRESS;
  transfermap[target.get()] = id;
  target->engageList(sl, connid, hiddenfiles);
  return id;
}

void TransferManager::suggestTransfer(std::string name, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld) {
  suggestTransfer(name, src, fls, name, dst, fld);
}

void TransferManager::suggestTransfer(std::string srcname, SiteLogic * src, FileList * fls, std::string dstname, SiteLogic * dst, FileList * fld) {
  Pointer<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageFXP(srcname, src, fls, dstname, dst, fld);
}

void TransferManager::suggestDownload(std::string name, SiteLogic * sl, FileList * filelist, std::string path) {
  Pointer<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageDownload(name, sl, filelist, path);
}

void TransferManager::suggestUpload(std::string name, std::string path, SiteLogic * sl, FileList * filelist) {
  Pointer<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageUpload(name, path, sl, filelist);
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
    target = Pointer<TransferMonitor>(new TransferMonitor(this));
    transfermonitors.push_back(target);
  }
  return target;
}

int TransferManager::transferStatus(int id) const {
  std::map<int, int>::const_iterator it = transferstatus.find(id);
  if (it != transferstatus.end()) {
    return it->second;
  }
  return 0;
}

void TransferManager::transferSuccessful(TransferMonitor * monitor) {
  bool push = false;
  if (transferstatus[transfermap[monitor]] == TRANSFER_IN_PROGRESS_UI) {
    push = true;
  }
  transferstatus[transfermap[monitor]] = TRANSFER_SUCCESSFUL;
  if (push) {
    global->getUIBase()->backendPush();
  }
  Pointer<TransferStatus> ts = monitor->getTransferStatus();
  if (!!ts) {
    moveTransferStatusToFinished(ts);
  }
}

void TransferManager::transferFailed(TransferMonitor * monitor, int err) {
  bool push = false;
  if (transferstatus[transfermap[monitor]] == TRANSFER_IN_PROGRESS_UI) {
    push = true;
  }
  transferstatus[transfermap[monitor]] = TRANSFER_FAILED;
  if (push) {
    global->getUIBase()->backendPush();
  }
  Pointer<TransferStatus> ts = monitor->getTransferStatus();
  if (!!ts) {
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

void TransferManager::addNewTransferStatus(Pointer<TransferStatus> ts) {
  ongoingtransfers.push_front(ts);
}

void TransferManager::moveTransferStatusToFinished(Pointer<TransferStatus> movets) {
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
