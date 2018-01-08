#pragma once

#include <list>
#include <string>
#include <map>

#include "core/pointer.h"

class CommandOwner;
class ScoreBoardElement;
class TransferMonitor;
class TransferStatus;
class SiteLogic;
class FileList;
class TransferStatus;
class LocalFileList;

#define MAX_TRANSFER_HISTORY 10000

class TransferManager {
  private:
    std::list<Pointer<TransferMonitor> > transfermonitors;
    std::list<Pointer<TransferStatus> > ongoingtransfers;
    std::list<Pointer<TransferStatus> > finishedtransfers;
    unsigned int totalfinishedtransfers;
    Pointer<TransferMonitor> getAvailableTransferMonitor();
    void moveTransferStatusToFinished(const Pointer<TransferStatus> &);
  public:
    TransferManager();
    ~TransferManager();
    void getFileList(const Pointer<SiteLogic> &, int, bool, FileList *, CommandOwner *);
    Pointer<TransferStatus> suggestTransfer(
      const std::string &, const Pointer<SiteLogic> &, FileList *,
      const Pointer<SiteLogic> &, FileList *, CommandOwner *, CommandOwner *);
    Pointer<TransferStatus> suggestTransfer(
      const std::string &, const Pointer<SiteLogic> &, FileList *,
      const std::string &, const Pointer<SiteLogic> &, FileList *,
      CommandOwner *, CommandOwner *);
    Pointer<TransferStatus> suggestDownload(
      const std::string &, const Pointer<SiteLogic> &, FileList *,
      const Pointer<LocalFileList> &, CommandOwner *);
    Pointer<TransferStatus> suggestUpload(
      const std::string &, const Pointer<LocalFileList> &,
      const Pointer<SiteLogic> &, FileList *, CommandOwner *);
    void transferSuccessful(const Pointer<TransferStatus> &);
    void transferFailed(const Pointer<TransferStatus> &, int);
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator ongoingTransfersEnd() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersBegin() const;
    std::list<Pointer<TransferStatus> >::const_iterator finishedTransfersEnd() const;
    unsigned int ongoingTransfersSize() const;
    unsigned int finishedTransfersSize() const;
    unsigned int totalFinishedTransfers() const;
    void addNewTransferStatus(const Pointer<TransferStatus> &);
};
