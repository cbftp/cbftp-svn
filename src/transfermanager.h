#pragma once

#include <list>
#include <string>
#include <map>

#define TRANSFER_IN_PROGRESS 3365
#define TRANSFER_SUCCESSFUL 3366
#define TRANSFER_FAILED 3367
#define TRANSFER_IN_PROGRESS_UI 3365

#define MAX_TRANSFER_HISTORY 100

class ScoreBoardElement;
class TransferMonitor;
class TransferStatus;
class SiteLogic;
class FileList;

class TransferManager {
  private:
    std::list<TransferMonitor *> transfersmonitors;
    std::list<TransferStatus *> ongoingtransfers;
    std::list<TransferStatus *> finishedtransfers;
    int requestids;
    std::map<TransferMonitor*, int> transfermap;
    std::map<int, int> transferstatus;
    TransferMonitor * getAvailableTransferMonitor();
    void moveTransferStatusToFinished(TransferStatus *);
  public:
    TransferManager();
    int download(std::string, SiteLogic *, FileList *);
    int getFileList(SiteLogic *, int);
    void suggestTransfer(std::string, SiteLogic *, FileList *, SiteLogic *, FileList *);
    void suggestTransfer(std::string, SiteLogic *, FileList *, std::string, SiteLogic *, FileList *);
    int transferStatus(int);
    void transferSuccessful(TransferMonitor *);
    void transferFailed(TransferMonitor *, int);
    std::list<TransferStatus *>::iterator ongoingTransfersBegin();
    std::list<TransferStatus *>::iterator ongoingTransfersEnd();
    std::list<TransferStatus *>::iterator finishedTransfersBegin();
    std::list<TransferStatus *>::iterator finishedTransfersEnd();
    void addNewTransferStatus(TransferStatus *);
};
