#pragma once

#include <list>
#include <string>
#include <map>

#include "pointer.h"

#define MAX_TRANSFER_HISTORY 100

class ScoreBoardElement;
class TransferMonitor;
class TransferStatus;
class SiteLogic;
class FileList;
class TransferStatus;

class TransferManager {
  private:
    std::list<Pointer<TransferMonitor> > transfermonitors;
    std::list<Pointer<TransferStatus> > ongoingtransfers;
    std::list<Pointer<TransferStatus> > finishedtransfers;
    Pointer<TransferMonitor> getAvailableTransferMonitor();
    void moveTransferStatusToFinished(Pointer<TransferStatus>);
  public:
    TransferManager();
    void getFileList(SiteLogic *, int, bool);
    void suggestTransfer(std::string, SiteLogic *, FileList *, SiteLogic *, FileList *);
    void suggestTransfer(std::string, SiteLogic *, FileList *, std::string, SiteLogic *, FileList *);
    Pointer<TransferStatus> suggestDownload(std::string, SiteLogic *, FileList *, std::string);
    void suggestUpload(std::string, std::string, SiteLogic *, FileList *);
    void transferSuccessful(Pointer<TransferStatus>);
    void transferFailed(Pointer<TransferStatus>, int);
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersEnd() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersEnd() const;
    void addNewTransferStatus(Pointer<TransferStatus>);
};
