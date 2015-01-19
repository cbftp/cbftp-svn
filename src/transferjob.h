#pragma once

#include <string>
#include <map>

#include "commandowner.h"

#define TRANSFERJOB_DOWNLOAD 2161
#define TRANSFERJOB_DOWNLOAD_FILE 2162
#define TRANSFERJOB_UPLOAD 2163
#define TRANSFERJOB_UPLOAD_FILE 2164
#define TRANSFERJOB_FXP 2165
#define TRANSFERJOB_FXP_FILE 2166

class SiteLogic;
class FileList;

class TransferJob : public CommandOwner {
public:
  int classType() const;
  TransferJob(SiteLogic *, std::string, FileList *, std::string, std::string);
  TransferJob(std::string, std::string, SiteLogic *, std::string, FileList *);
  TransferJob(SiteLogic *, std::string, FileList *, SiteLogic *, std::string, FileList *);
  std::string getSrcFileName() const;
  std::string getDstFileName() const;
  int getType() const;
  std::string getSrcPath() const;
  std::string getDstPath() const;
  FileList * getSrcFileList() const;
  FileList * getDstFileList() const;
  std::map<std::string, FileList *>::const_iterator srcFileListsBegin() const;
  std::map<std::string, FileList *>::const_iterator srcFileListsEnd() const;
  std::map<std::string, FileList *>::const_iterator dstFileListsBegin() const;
  std::map<std::string, FileList *>::const_iterator dstFileListsEnd() const;
  bool isDone() const;
  bool wantsList(SiteLogic *);
  FileList * getListTarget(SiteLogic *) const;
  void fileListUpdated(FileList *);
  SiteLogic * getSrc() const;
  SiteLogic * getDst() const;
  int maxSlots() const;
  void setSlots(int);
  bool listsRefreshed() const;
  void refreshLists();
  void setDone();
  void clearRefreshLists();
private:
  void addSubDirectoryFileLists(std::map<std::string, FileList *> &, FileList *);
  void init();
  int type;
  SiteLogic * src;
  SiteLogic * dst;
  std::string srcfile;
  std::string dstfile;
  std::string srcpath;
  std::string dstpath;
  FileList * srclist;
  FileList * dstlist;
  std::map<std::string, FileList *> srcfilelists;
  std::map<std::string, FileList *> dstfilelists;
  int slots;
  bool done;
  bool listsrefreshed;
  FileList * srclisttarget;
  FileList * dstlisttarget;
  std::map<FileList *, bool> filelistsrefreshed;
};
