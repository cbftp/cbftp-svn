#include "transferjob.h"

#include "filelist.h"
#include "file.h"
#include "site.h"
#include "globalcontext.h"
#include "skiplist.h"

extern GlobalContext * global;

TransferJob::TransferJob(SiteLogic * sl, std::string srcfile, FileList * filelist, std::string path, std::string dstfile) :
      src(sl),
      dst(NULL),
      srcfile(srcfile),
      dstfile(dstfile),
      dstpath(path),
      srclist(filelist) {
  init();
  if (srclist->getFile(srcfile)->isDirectory()) {
    type = TRANSFERJOB_DOWNLOAD;
    srcfilelists[""] = new FileList(filelist->getUser(), filelist->getPath() + "/" + srcfile);
  }
  else {
    type = TRANSFERJOB_DOWNLOAD_FILE;
  }
}

TransferJob::TransferJob(std::string path, std::string srcfile, SiteLogic * sl, std::string dstfile, FileList * filelist) :
      src(NULL),
      dst(sl),
      srcfile(srcfile),
      dstfile(dstfile),
      srcpath(path),
      dstlist(filelist) {
  init();
  type = TRANSFERJOB_UPLOAD;
}

TransferJob::TransferJob(SiteLogic * slsrc, std::string srcfile, FileList * srcfilelist, SiteLogic * sldst, std::string dstfile, FileList * dstfilelist) :
      src(slsrc),
      dst(sldst),
      srcfile(srcfile),
      dstfile(dstfile),
      srclist(srcfilelist),
      dstlist(dstfilelist) {
  init();
  if (srclist->getFile(srcfile)->isDirectory()) {
    type = TRANSFERJOB_FXP;
    srcfilelists[""] = new FileList(srcfilelist->getUser(), srcfilelist->getPath() + "/" + srcfile);
    dstfilelists[""] = new FileList(dstfilelist->getUser(), dstfilelist->getPath() + "/" + dstfile);
  }
  else {
    type = TRANSFERJOB_FXP_FILE;
  }
}

int TransferJob::classType() const {
  return COMMANDOWNER_TRANSFERJOB;
}

std::string TransferJob::getSrcPath() const {
  return srcpath;
}

std::string TransferJob::getDstPath() const {
  return dstpath;
}

FileList * TransferJob::getSrcFileList() const {
  return srclist;
}

FileList * TransferJob::getDstFileList() const {
  return dstlist;
}

std::map<std::string, FileList *>::const_iterator TransferJob::srcFileListsBegin() const {
  return srcfilelists.begin();
}

std::map<std::string, FileList *>::const_iterator TransferJob::srcFileListsEnd() const {
  return srcfilelists.end();
}

std::map<std::string, FileList *>::const_iterator TransferJob::dstFileListsBegin() const {
  return dstfilelists.begin();
}

std::map<std::string, FileList *>::const_iterator TransferJob::dstFileListsEnd() const {
  return dstfilelists.end();
}

void TransferJob::init() {
  done = false;
  listsrefreshed = false;
  slots = 1;
  srclisttarget = NULL;
  dstlisttarget = NULL;
}

std::string TransferJob::getSrcFileName() const {
  return srcfile;
}

std::string TransferJob::getDstFileName() const {
  return dstfile;
}

int TransferJob::getType() const {
  return type;
}

bool TransferJob::isDone() const {
  return done;
}

bool TransferJob::wantsList(SiteLogic * sl) {
  if ((type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP) && sl == src) {
    for (std::map<std::string, FileList *>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
      std::map<FileList *, bool>::iterator it2 = filelistsrefreshed.find(it->second);
      if (!it->second->isFilled() || (it2 != filelistsrefreshed.end() && !it2->second)) {
        srclisttarget = it->second;
        return true;
      }
    }
  }
  return false;
}

FileList * TransferJob::getListTarget(SiteLogic * sl) const {
  if (sl == src) {
    return srclisttarget;
  }
  else if (sl == dst) {
    return dstlisttarget;
  }
  return NULL;
}

void TransferJob::fileListUpdated(FileList * fl) {
  std::map<FileList *, bool>::iterator it2 = filelistsrefreshed.find(fl);
  if (it2 != filelistsrefreshed.end()) {
    it2->second = true;
    bool allrefreshed = true;
    for (it2 = filelistsrefreshed.begin(); it2 != filelistsrefreshed.end(); it2++) {
      if (!it2->second) {
        allrefreshed = false;
        break;
      }
    }
    if (allrefreshed) {
      listsrefreshed = true;
    }
  }
  if (type != TRANSFERJOB_UPLOAD) {
    for (std::map<std::string, FileList *>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
      if (it->second == fl) {
        addSubDirectoryFileLists(srcfilelists, fl);
        return;
      }
    }
  }
  for (std::map<std::string, FileList *>::iterator it = dstfilelists.begin(); it != dstfilelists.end(); it++) {
    if (it->second == fl) {
      addSubDirectoryFileLists(dstfilelists, fl);
      return;
    }
  }
}

void TransferJob::addSubDirectoryFileLists(std::map<std::string, FileList *> & filelists, FileList * filelist) {
  std::map<std::string, File *>::iterator it;
  for(it = filelist->begin(); it != filelist->end(); it++) {
    File * file = it->second;
    if (file->isDirectory()) {
      if (!global->getSkipList()->isAllowed(it->first, true, false)) {
        continue;
      }
      std::string filename = file->getName();
      if (filelists.find(filename) == filelists.end()) {
        filelists[filename] = new FileList(filelist->getUser(), filelist->getPath() + "/" + filename);
      }
    }
  }
}

SiteLogic * TransferJob::getSrc() const {
  return src;
}

SiteLogic * TransferJob::getDst() const {
  return dst;
}

int TransferJob::maxSlots() const {
  return slots;
}

void TransferJob::setSlots(int slots) {
  this->slots = slots;
}

bool TransferJob::listsRefreshed() const {
  return listsrefreshed;
}

void TransferJob::refreshLists() {
  clearRefreshLists();
  for (std::map<std::string, FileList *>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
    filelistsrefreshed[it->second] = false;
  }
  for (std::map<std::string, FileList *>::iterator it = dstfilelists.begin(); it != dstfilelists.end(); it++) {
    filelistsrefreshed[it->second] = false;
  }
}

void TransferJob::setDone() {
  done = true;
}

void TransferJob::clearRefreshLists() {
  filelistsrefreshed.clear();
  listsrefreshed = false;
}
