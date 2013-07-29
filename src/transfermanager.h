#pragma once

#include <string>

#include "globalcontext.h"
#include "sitelogic.h"
#include "scoreboardelement.h"
#include "transfermonitor.h"

extern GlobalContext * global;

class TransferManager {
  private:
    std::list<TransferMonitor *> transfers;
  public:
    TransferManager();
    void suggestTransfer(ScoreBoardElement *);
};
