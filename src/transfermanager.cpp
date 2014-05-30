#include "transfermanager.h"

#include "scoreboardelement.h"
#include "transfermonitor.h"
#include "uibase.h"
#include "globalcontext.h"

extern GlobalContext * global;

TransferManager::TransferManager() {
  requestids = 0;
}

int TransferManager::download(std::string name, SiteLogic * sl, FileList * filelist) {
  int id = requestids++;
  TransferMonitor * target = getAvailableTransferMonitor();
  transferstatus[id] = TRANSFER_IN_PROGRESS_UI;
  transfermap[target] = id;
  target->engage(name, sl, filelist);
  return id;
}

int TransferManager::getFileList(SiteLogic * sl, int connid) {
  int id = requestids++;
  TransferMonitor * target = getAvailableTransferMonitor();
  transferstatus[id] = TRANSFER_IN_PROGRESS;
  transfermap[target] = id;
  target->engage(sl, connid);
  return id;
}

void TransferManager::suggestTransfer(std::string name, SiteLogic * src, FileList * fls, SiteLogic * dst, FileList * fld) {
  suggestTransfer(name, src, fls, name, dst, fld);
}

void TransferManager::suggestTransfer(std::string srcname, SiteLogic * src, FileList * fls, std::string dstname, SiteLogic * dst, FileList * fld) {
  TransferMonitor * target = getAvailableTransferMonitor();
  target->engage(srcname, src, fls, dstname, dst, fld);
}

TransferMonitor * TransferManager::getAvailableTransferMonitor() {
  TransferMonitor * target = NULL;
  std::list<TransferMonitor *>::iterator it;
  for (it = transfers.begin(); it != transfers.end(); it++) {
    if ((*it)->idle()) {
      target = *it;
      break;
    }
  }
  if (target == NULL) {
    target = new TransferMonitor(this);
    transfers.push_back(target);
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
}
