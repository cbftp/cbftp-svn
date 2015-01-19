#pragma once

#include <list>
#include <string>
#include <map>

#include "pointer.h"

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
    std::list<Pointer<TransferMonitor> > transfermonitors;
    std::list<Pointer<TransferStatus> > ongoingtransfers;
    std::list<Pointer<TransferStatus> > finishedtransfers;
    int requestids;
    std::map<TransferMonitor *, int> transfermap;
    std::map<int, int> transferstatus;
    Pointer<TransferMonitor> getAvailableTransferMonitor();
    void moveTransferStatusToFinished(Pointer<TransferStatus>);
  public:
    TransferManager();
    int download(std::string, SiteLogic *, FileList *, std::string);
    int getFileList(SiteLogic *, int, bool);
    void suggestTransfer(std::string, SiteLogic *, FileList *, SiteLogic *, FileList *);
    void suggestTransfer(std::string, SiteLogic *, FileList *, std::string, SiteLogic *, FileList *);
    void suggestDownload(std::string, SiteLogic *, FileList *, std::string);
    void suggestUpload(std::string, std::string, SiteLogic *, FileList *);
    int transferStatus(int) const;
    void transferSuccessful(TransferMonitor *);
    void transferFailed(TransferMonitor *, int);
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersEnd() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersEnd() const;
    void addNewTransferStatus(Pointer<TransferStatus>);
};
