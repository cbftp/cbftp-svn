#pragma once

#include <list>
#include <unordered_map>
#include <memory>
#include <unordered_set>
#include <string>

#include "core/eventreceiver.h"
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
  TransferJob(unsigned int, const std::shared_ptr<SiteLogic> &, FileList *, const std::string &, const Path &, const std::string &);
  TransferJob(unsigned int, const std::shared_ptr<SiteLogic> &, const Path &, const std::string &, const Path &, const std::string &);
  TransferJob(unsigned int, const Path &, const std::string &, const std::shared_ptr<SiteLogic> &, FileList *, const std::string &);
  TransferJob(unsigned int, const Path &, const std::string &, const std::shared_ptr<SiteLogic> &, const Path &, const std::string &);
  TransferJob(unsigned int, const std::shared_ptr<SiteLogic> &, FileList *, const std::string &, const std::shared_ptr<SiteLogic> &, FileList *, const std::string &);
  TransferJob(unsigned int, const std::shared_ptr<SiteLogic> &, const Path &, const std::string &, const std::shared_ptr<SiteLogic> &, const Path &, const std::string &);
  ~TransferJob();
  CallbackType callbackType() const override;
  int getType() const;
  const Path & getSrcPath() const;
  const Path & getDstPath() const;
  const Path & getPath(bool source) const;
  std::string getSrcFileName() const;
  std::string getDstFileName() const;
  std::unordered_map<std::string, FileList *>::const_iterator srcFileListsBegin() const;
  std::unordered_map<std::string, FileList *>::const_iterator srcFileListsEnd() const;
  std::unordered_map<std::string, FileList *>::const_iterator dstFileListsBegin() const;
  std::unordered_map<std::string, FileList *>::const_iterator dstFileListsEnd() const;
  std::unordered_map<std::string, std::shared_ptr<LocalFileList> >::const_iterator localFileListsBegin() const;
  std::unordered_map<std::string, std::shared_ptr<LocalFileList> >::const_iterator localFileListsEnd() const;
  std::list<std::shared_ptr<TransferStatus> >::const_iterator transfersBegin() const;
  std::list<std::shared_ptr<TransferStatus> >::const_iterator transfersEnd() const;
  std::unordered_map<std::string, unsigned long long int>::const_iterator pendingTransfersBegin() const;
  std::unordered_map<std::string, unsigned long long int>::const_iterator pendingTransfersEnd() const;
  bool isDone() const;
  TransferJobStatus getStatus() const;
  bool wantsList(bool source);
  std::shared_ptr<LocalFileList> wantedLocalDstList(const std::string &);
  FileList * getListTarget(bool source) const;
  void fileListUpdated(bool source, FileList *);
  FileList * findDstList(const std::string &) const;
  FileList * getFileListForFullPath(bool source, const Path &) const;
  std::shared_ptr<LocalFileList> findLocalFileList(const std::string &) const;
  const std::shared_ptr<SiteLogic> & getSrc() const;
  const std::shared_ptr<SiteLogic> & getDst() const;
  int maxSlots() const;
  void setSlots(int);
  int maxPossibleSlots() const;
  bool listsRefreshed() const;
  bool refreshOrAlmostDone();
  void clearRefreshLists();
  void start();
  void addPendingTransfer(const Path &, unsigned long long int);
  void addTransfer(const std::shared_ptr<TransferStatus> &);
  void targetExists(const Path &);
  void tick(int);
  int getProgress() const;
  int getMilliProgress() const;
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
  void transferSuccessful(const std::shared_ptr<TransferStatus> &);
  void transferFailed(const std::shared_ptr<TransferStatus> &, int);
  bool anyListNeedsRefreshing() const;
  std::shared_ptr<SiteTransferJob> & getSrcTransferJob();
  std::shared_ptr<SiteTransferJob> & getDstTransferJob();
  void updateStatus();
private:
  void downloadJob(unsigned int, const std::shared_ptr<SiteLogic> &, FileList *, const std::string &, const Path &, const std::string &);
  void uploadJob(unsigned int, const Path &, const std::string &, const std::shared_ptr<SiteLogic> &, FileList *, const std::string &);
  void fxpJob(unsigned int, const std::shared_ptr<SiteLogic> &, FileList *, const std::string &, const std::shared_ptr<SiteLogic> &, FileList *, const std::string &);
  void addTransferAttempt(const std::shared_ptr<TransferStatus> &, bool);
  void addSubDirectoryFileLists(std::unordered_map<std::string, FileList *> &, FileList *, const Path &);
  void addSubDirectoryFileLists(std::unordered_map<std::string, FileList *> &, FileList *, const Path &, File *);
  void init(unsigned int, TransferJobType, const std::shared_ptr<SiteLogic> &, const std::shared_ptr<SiteLogic> &, const Path &, const Path &, const std::string &, const std::string &);
  void countTotalFiles();
  void setDone();
  void updateLocalFileLists();
  void updateLocalFileLists(const Path &, const Path &);
  void checkFileListExists(FileList *) const;
  int type;
  std::shared_ptr<SiteLogic> src;
  std::shared_ptr<SiteLogic> dst;
  Path srcpath;
  Path dstpath;
  std::string srcfile;
  std::string dstfile;
  std::unordered_map<std::string, FileList *> srcfilelists;
  std::unordered_map<std::string, FileList *> dstfilelists;
  std::unordered_map<std::string, std::shared_ptr<LocalFileList> > localfilelists;
  std::unordered_map<std::string, unsigned long long int> pendingtransfers;
  std::unordered_set<std::string> existingtargets;
  std::list<std::shared_ptr<TransferStatus> > transfers;
  int slots;
  TransferJobStatus status;
  FileList * srclisttarget;
  FileList * dstlisttarget;
  std::unordered_map<FileList *, int> filelistsrefreshed;
  unsigned long long int expectedfinalsize;
  unsigned int speed;
  unsigned long long int sizeprogress;
  int progress;
  int milliprogress;
  int timespentmillis;
  int timespentsecs;
  int timeremaining;
  std::string timequeued;
  std::string timestarted;
  int filesprogress;
  int filestotal;
  bool almostdone;
  unsigned int id;
  std::unordered_map<std::string, int> transferattempts;
  int idletime;
  std::shared_ptr<SiteTransferJob> srcsitetransferjob;
  std::shared_ptr<SiteTransferJob> dstsitetransferjob;
};
