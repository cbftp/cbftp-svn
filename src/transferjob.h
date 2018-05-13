#pragma once

#include <string>
#include <map>
#include <set>
#include <list>

#include "core/eventreceiver.h"
#include "core/pointer.h"
#include "transferstatuscallback.h"
#include "path.h"

enum TransferJobType {
  TRANSFERJOB_DOWNLOAD,
  TRANSFERJOB_UPLOAD,
  TRANSFERJOB_FXP
};

enum TransferJobStatus {
  TRANSFERJOB_QUEUED,
  TRANSFERJOB_RUNNING,
  TRANSFERJOB_DONE,
  TRANSFERJOB_ABORTED
};

#define TRANSFERJOB_UPDATE_INTERVAL 250

class SiteTransferJob;
class SiteLogic;
class FileList;
class TransferStatus;
class LocalFileList;
class File;

class TransferJob : public EventReceiver, public TransferStatusCallback {
public:
  std::string getName() const;
  TransferJob(unsigned int, const Pointer<SiteLogic> &, FileList *, const std::string &, const Path &, const std::string &);
  TransferJob(unsigned int, const Pointer<SiteLogic> &, const Path &, const std::string &, const Path &, const std::string &);
  TransferJob(unsigned int, const Path &, const std::string &, const Pointer<SiteLogic> &, FileList *, const std::string &);
  TransferJob(unsigned int, const Path &, const std::string &, const Pointer<SiteLogic> &, const Path &, const std::string &);
  TransferJob(unsigned int, const Pointer<SiteLogic> &, FileList *, const std::string &, const Pointer<SiteLogic> &, FileList *, const std::string &);
  TransferJob(unsigned int, const Pointer<SiteLogic> &, const Path &, const std::string &, const Pointer<SiteLogic> &, const Path &, const std::string &);
  ~TransferJob();
  int getType() const;
  const Path & getSrcPath() const;
  const Path & getDstPath() const;
  const Path & getPath(bool source) const;
  std::string getSrcFileName() const;
  std::string getDstFileName() const;
  std::map<std::string, FileList *>::const_iterator srcFileListsBegin() const;
  std::map<std::string, FileList *>::const_iterator srcFileListsEnd() const;
  std::map<std::string, FileList *>::const_iterator dstFileListsBegin() const;
  std::map<std::string, FileList *>::const_iterator dstFileListsEnd() const;
  std::map<std::string, Pointer<LocalFileList> >::const_iterator localFileListsBegin() const;
  std::map<std::string, Pointer<LocalFileList> >::const_iterator localFileListsEnd() const;
  std::list<Pointer<TransferStatus> >::const_iterator transfersBegin() const;
  std::list<Pointer<TransferStatus> >::const_iterator transfersEnd() const;
  std::map<std::string, unsigned long long int>::const_iterator pendingTransfersBegin() const;
  std::map<std::string, unsigned long long int>::const_iterator pendingTransfersEnd() const;
  bool isDone() const;
  TransferJobStatus getStatus() const;
  bool wantsList(bool source);
  Pointer<LocalFileList> wantedLocalDstList(const std::string &);
  FileList * getListTarget(bool source) const;
  void fileListUpdated(bool source, FileList *);
  FileList * findDstList(const std::string &) const;
  FileList * getFileListForFullPath(bool source, const Path &) const;
  Pointer<LocalFileList> findLocalFileList(const std::string &) const;
  const Pointer<SiteLogic> & getSrc() const;
  const Pointer<SiteLogic> & getDst() const;
  int maxSlots() const;
  void setSlots(int);
  int maxPossibleSlots() const;
  bool listsRefreshed() const;
  bool refreshOrAlmostDone();
  void clearRefreshLists();
  void start();
  void addPendingTransfer(const Path &, unsigned long long int);
  void addTransfer(const Pointer<TransferStatus> &);
  void targetExists(const Path &);
  void tick(int);
  int getProgress() const;
  int timeSpent() const;
  int timeRemaining() const;
  unsigned long long int sizeProgress() const;
  unsigned long long int totalSize() const;
  unsigned int getSpeed() const;
  std::string timeQueued() const;
  std::string timeStarted() const;
  std::string typeString() const;
  int filesProgress() const;
  int filesTotal() const;
  unsigned int getId() const;
  void abort();
  void clearExisting();
  bool hasFailedTransfer(const std::string &) const;
  void transferSuccessful(const Pointer<TransferStatus> &);
  void transferFailed(const Pointer<TransferStatus> &, int);
  bool anyListNeedsRefreshing() const;
  Pointer<SiteTransferJob> & getSrcTransferJob();
  Pointer<SiteTransferJob> & getDstTransferJob();
  void updateStatus();
private:
  void downloadJob(unsigned int, const Pointer<SiteLogic> &, FileList *, const std::string &, const Path &, const std::string &);
  void uploadJob(unsigned int, const Path &, const std::string &, const Pointer<SiteLogic> &, FileList *, const std::string &);
  void fxpJob(unsigned int, const Pointer<SiteLogic> &, FileList *, const std::string &, const Pointer<SiteLogic> &, FileList *, const std::string &);
  void addTransferAttempt(const Pointer<TransferStatus> &, bool);
  void addSubDirectoryFileLists(std::map<std::string, FileList *> &, FileList *, const Path &);
  void addSubDirectoryFileLists(std::map<std::string, FileList *> &, FileList *, const Path &, File *);
  void init(unsigned int, TransferJobType, const Pointer<SiteLogic> &, const Pointer<SiteLogic> &, const Path &, const Path &, const std::string &, const std::string &);
  void countTotalFiles();
  void setDone();
  void updateLocalFileLists();
  void updateLocalFileLists(const Path &, const Path &);
  void checkFileListExists(FileList *) const;
  int type;
  Pointer<SiteLogic> src;
  Pointer<SiteLogic> dst;
  Path srcpath;
  Path dstpath;
  std::string srcfile;
  std::string dstfile;
  std::map<std::string, FileList *> srcfilelists;
  std::map<std::string, FileList *> dstfilelists;
  std::map<std::string, Pointer<LocalFileList> > localfilelists;
  std::map<std::string, unsigned long long int> pendingtransfers;
  std::set<std::string> existingtargets;
  std::list<Pointer<TransferStatus> > transfers;
  int slots;
  TransferJobStatus status;
  FileList * srclisttarget;
  FileList * dstlisttarget;
  std::map<FileList *, int> filelistsrefreshed;
  unsigned long long int expectedfinalsize;
  unsigned int speed;
  unsigned long long int sizeprogress;
  int progress;
  int timespentmillis;
  int timespentsecs;
  int timeremaining;
  std::string timequeued;
  std::string timestarted;
  int filesprogress;
  int filestotal;
  bool almostdone;
  unsigned int id;
  std::map<std::string, int> transferattempts;
  int idletime;
  Pointer<SiteTransferJob> srcsitetransferjob;
  Pointer<SiteTransferJob> dstsitetransferjob;
};
