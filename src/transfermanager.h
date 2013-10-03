#pragma once

#include <list>

class ScoreBoardElement;
class TransferMonitor;
class SiteLogic;

class TransferManager {
  private:
    std::list<TransferMonitor *> transfers;
  public:
    TransferManager();
    void suggestTransfer(ScoreBoardElement *);
};
