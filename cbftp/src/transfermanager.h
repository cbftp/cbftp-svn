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
    void moveTransferStatusToFinished(const Pointer<TransferStatus> &);
  public:
    TransferManager();
    ~TransferManager();
    void getFileList(const Pointer<SiteLogic> &, int, bool);
    Pointer<TransferStatus> suggestTransfer(const std::string &, const Pointer<SiteLogic> &, FileList *, const Pointer<SiteLogic> &, FileList *);
    Pointer<TransferStatus> suggestTransfer(const std::string &, const Pointer<SiteLogic> &, FileList *, const std::string &, const Pointer<SiteLogic> &, FileList *);
    Pointer<TransferStatus> suggestDownload(const std::string &, const Pointer<SiteLogic> &, FileList *, const Pointer<LocalFileList> &);
    Pointer<TransferStatus> suggestUpload(const std::string &, const Pointer<LocalFileList> &, const Pointer<SiteLogic> &, FileList *);
    void transferSuccessful(const Pointer<TransferStatus> &);
    void transferFailed(const Pointer<TransferStatus> &, int);
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersEnd() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersEnd() const;
    unsigned int ongoingTransfersSize() const;
    unsigned int finishedTransfersSize() const;
    void addNewTransferStatus(const Pointer<TransferStatus> &);
};
