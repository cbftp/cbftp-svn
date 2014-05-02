#pragma once

#include <list>
#include <string>
#include <map>

#define TRANSFER_IN_PROGRESS 3365
#define TRANSFER_SUCCESSFUL 3366
#define TRANSFER_FAILED 3367
#define TRANSFER_IN_PROGRESS_UI 3365

class ScoreBoardElement;
class TransferMonitor;
class SiteLogic;
class FileList;

class TransferManager {
  private:
    std::list<TransferMonitor *> transfers;
    int requestids;
    std::map<TransferMonitor*, int> transfermap;
    std::map<int, int> transferstatus;
    TransferMonitor * getAvailableTransferMonitor();
  public:
    TransferManager();
    int download(std::string, SiteLogic *, FileList *);
    int getFileList(SiteLogic *, int);
    void suggestTransfer(std::string, SiteLogic *, FileList *, SiteLogic *, FileList *);
    void suggestTransfer(std::string, SiteLogic *, FileList *, std::string, SiteLogic *, FileList *);
    int transferStatus(int);
    void transferSuccessful(TransferMonitor *);
    void transferFailed(TransferMonitor *, int);
};
