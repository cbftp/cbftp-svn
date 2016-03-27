#pragma once

#include <list>
#include <string>
#include <map>

#include "core/pointer.h"

class ScoreBoardElement;
class TransferMonitor;
class TransferStatus;
class SiteLogic;
class FileList;
class TransferStatus;
class LocalFileList;

class TransferManager {
  private:
    std::list<Pointer<TransferMonitor> > transfermonitors;
    std::list<Pointer<TransferStatus> > ongoingtransfers;
    std::list<Pointer<TransferStatus> > finishedtransfers;
    Pointer<TransferMonitor> getAvailableTransferMonitor();
    void moveTransferStatusToFinished(Pointer<TransferStatus> &);
  public:
    TransferManager();
    ~TransferManager();
    void getFileList(SiteLogic *, int, bool);
    Pointer<TransferStatus> suggestTransfer(std::string, SiteLogic *, FileList *, SiteLogic *, FileList *);
    Pointer<TransferStatus> suggestTransfer(std::string, SiteLogic *, FileList *, std::string, SiteLogic *, FileList *);
    Pointer<TransferStatus> suggestDownload(std::string, SiteLogic *, FileList *, Pointer<LocalFileList> &);
    Pointer<TransferStatus> suggestUpload(std::string, Pointer<LocalFileList> &, SiteLogic *, FileList *);
    void transferSuccessful(Pointer<TransferStatus> &);
    void transferFailed(Pointer<TransferStatus> &, int);
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersEnd() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersEnd() const;
    void addNewTransferStatus(Pointer<TransferStatus> &);
};
