#include "transfermanager.h"

TransferManager::TransferManager() {

}

void TransferManager::suggestTransfer(ScoreBoardElement * sbe) {
  std::string name = sbe->fileName();
  SiteLogic * src = sbe->getSource();
  SiteLogic * dst = sbe->getDestination();
  FileList * fls = sbe->getSourceFileList();
  FileList * fld = sbe->getDestinationFileList();
  TransferMonitor * target = NULL;
  std::list<TransferMonitor *>::iterator it;
  for (it = transfers.begin(); it != transfers.end(); it++) {
    if ((*it)->idle()) {
      target = *it;
      break;
    }
  }
  if (target == NULL) {
    target = new TransferMonitor();
    transfers.push_back(target);
  }
  target->engage(name, src, fls, dst, fld);
}
