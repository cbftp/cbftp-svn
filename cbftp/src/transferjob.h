#pragma once

#include <string>
#include <map>
#include <set>
#include <list>

#include "core/pointer.h"
#include "commandowner.h"

#define TRANSFERJOB_DOWNLOAD 2161
#define TRANSFERJOB_DOWNLOAD_FILE 2162
#define TRANSFERJOB_UPLOAD 2163
#define TRANSFERJOB_UPLOAD_FILE 2164
#define TRANSFERJOB_FXP 2165
#define TRANSFERJOB_FXP_FILE 2166

#define TRANSFERJOB_UPDATE_INTERVAL 250

class SiteLogic;
class FileList;
class TransferStatus;
class LocalFileList;

class TransferJob : public CommandOwner {
public:
  int classType() const;
  TransferJob(unsigned int, SiteLogic *, std::string, FileList *, std::string, std::string);
  TransferJob(unsigned int, std::string, std::string, SiteLogic *, std::string, FileList *);
  TransferJob(unsigned int, SiteLogic *, std::string, FileList *, SiteLogic *, std::string, FileList *);
  ~TransferJob();
  std::string getSrcFileName() const;
  std::string getDstFileName() const;
  int getType() const;
  std::string getLocalPath() const;
  FileList * getSrcFileList() const;
  FileList * getDstFileList() const;
  Pointer<LocalFileList> getLocalFileList() const;
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
  bool wantsList(SiteLogic *);
  bool wantsMakeDir(SiteLogic *) const;
  void wantDstDirectory(std::string);
  Pointer<LocalFileList> wantedLocalDstList(const std::string &);
  FileList * getListTarget(SiteLogic *) const;
  std::string getWantedMakeDir();
  void fileListUpdated(FileList *);
  FileList * findDstList(const std::string &) const;
  Pointer<LocalFileList> findLocalFileList(const std::string &) const;
  SiteLogic * getSrc() const;
  SiteLogic * getDst() const;
  int maxSlots() const;
  void setSlots(int);
  int maxPossibleSlots() const;
  bool listsRefreshed() const;
  void refreshOrAlmostDone();
  void clearRefreshLists();
  void addPendingTransfer(const std::string &, unsigned long long int);
  void addTransfer(const Pointer<TransferStatus> &);
  void targetExists(const std::string &);
  void tick(int);
  int getProgress() const;
  int timeSpent() const;
  int timeRemaining() const;
  unsigned long long int sizeProgress() const;
  unsigned long long int totalSize() const;
  unsigned int getSpeed() const;
  std::string timeStarted() const;
  std::string typeString() const;
  int filesProgress() const;
  int filesTotal() const;
  std::string findSubPath(const Pointer<TransferStatus> &) const;
  bool isInitialized() const;
  void setInitialized();
  bool isAborted() const;
  unsigned int getId() const;
  void abort();
  void clearExisting();
private:
  void addSubDirectoryFileLists(std::map<std::string, FileList *> &, FileList *);
  void updateStatus();
  void init();
  void countTotalFiles();
  void setDone();
  void checkRemoveWantedDstMakeDir(std::string);
  void updateLocalFileLists(const std::string &);
  int type;
  SiteLogic * src;
  SiteLogic * dst;
  std::string srcfile;
  std::string dstfile;
  std::string localpath;
  FileList * srclist;
  FileList * dstlist;
  Pointer<LocalFileList> locallist;
  std::map<std::string, FileList *> srcfilelists;
  std::map<std::string, FileList *> dstfilelists;
  std::map<std::string, Pointer<LocalFileList> > localfilelists;
  std::map<std::string, unsigned long long int> pendingtransfers;
  std::set<std::string> existingtargets;
  std::list<Pointer<TransferStatus> > transfers;
  int slots;
  bool almostdone;
  bool done;
  bool aborted;
  bool listsrefreshed;
  FileList * srclisttarget;
  FileList * dstlisttarget;
  std::map<FileList *, bool> filelistsrefreshed;
  unsigned long long int expectedfinalsize;
  unsigned int speed;
  unsigned long long int sizeprogress;
  int progress;
  int timespentmillis;
  int timespentsecs;
  int timeremaining;
  std::string timestarted;
  int filesprogress;
  int filestotal;
  bool initialized;
  std::set<std::string> wanteddstmakedirs;
  unsigned int id;
};
