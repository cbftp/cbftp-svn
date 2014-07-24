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

int TransferManager::download(std::string name, SiteLogic * sl, FileList * filelist) {
  int id = requestids++;
  TransferMonitor * target = getAvailableTransferMonitor();
  transferstatus[id] = TRANSFER_IN_PROGRESS_UI;
  transfermap[target] = id;
  target->engageDownload(name, sl, filelist);
  return id;
}

int TransferManager::getFileList(SiteLogic * sl, int connid, bool hiddenfiles) {
  int id = requestids++;
  TransferMonitor * target = getAvailableTransferMonitor();
  transferstatus[id] = TRANSFER_IN_PROGRESS;
  transfermap[target] = id;
  target->engageList(sl, connid, hiddenfiles);
  return id;
}

void TransferManager::suggestTransfer(std::string name, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld) {
  suggestTransfer(name, src, fls, name, dst, fld);
}

void TransferManager::suggestTransfer(std::string srcname, SiteLogic * src, FileList * fls, std::string dstname, SiteLogic * dst, FileList * fld) {
  TransferMonitor * target = getAvailableTransferMonitor();
  target->engageFXP(srcname, src, fls, dstname, dst, fld);
}

TransferMonitor * TransferManager::getAvailableTransferMonitor() {
  TransferMonitor * target = NULL;
  std::list<TransferMonitor *>::iterator it;
  for (it = transfersmonitors.begin(); it != transfersmonitors.end(); it++) {
    if ((*it)->idle()) {
      target = *it;
      break;
    }
  }
  if (target == NULL) {
    target = new TransferMonitor(this);
    transfersmonitors.push_back(target);
  }
  return target;
}

int TransferManager::transferStatus(int id) {
  return transferstatus[id];
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
  if (monitor->getTransferStatus() != NULL) {
    moveTransferStatusToFinished(monitor->getTransferStatus());
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
  if (monitor->getTransferStatus() != NULL) {
    moveTransferStatusToFinished(monitor->getTransferStatus());
  }
}

std::list<TransferStatus *>::iterator TransferManager::ongoingTransfersBegin() {
  return ongoingtransfers.begin();
}

std::list<TransferStatus *>::iterator TransferManager::ongoingTransfersEnd() {
  return ongoingtransfers.end();
}

std::list<TransferStatus *>::iterator TransferManager::finishedTransfersBegin() {
  return finishedtransfers.begin();
}

std::list<TransferStatus *>::iterator TransferManager::finishedTransfersEnd() {
  return finishedtransfers.end();
}

void TransferManager::addNewTransferStatus(TransferStatus * ts) {
  ongoingtransfers.push_front(ts);
}

void TransferManager::moveTransferStatusToFinished(TransferStatus * movets) {
  for (std::list<TransferStatus *>::iterator it = ongoingtransfers.begin(); it != ongoingtransfers.end(); it++) {
    if (*it == movets) {
      ongoingtransfers.erase(it);
      break;
    }
  }
  if (finishedtransfers.size() > MAX_TRANSFER_HISTORY) {
    TransferStatus * delts = finishedtransfers.back();
    finishedtransfers.pop_back();
    delete delts;
  }
  finishedtransfers.push_front(movets);
}
