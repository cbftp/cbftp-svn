#include "transferjob.h"

#include "core/tickpoke.h"
#include "filelist.h"
#include "file.h"
#include "site.h"
#include "globalcontext.h"
#include "skiplist.h"
#include "transferstatus.h"
#include "sitelogic.h"
#include "util.h"
#include "localstorage.h"
#include "localfilelist.h"
#include "localfile.h"
#include "filesystem.h"

TransferJob::TransferJob(unsigned int id, const Pointer<SiteLogic> & sl, std::string srcfile, FileList * filelist, const Path & path, std::string dstfile) :
      src(sl),
      dst(NULL),
      srcfile(srcfile),
      dstfile(dstfile),
      localpath(path),
      srclist(filelist),
      id(id)
{
  init();
  if (FileSystem::directoryExistsWritable(path)) {
    locallist = global->getLocalStorage()->getLocalFileList(path);
  }
  if (srclist->getFile(srcfile)->isDirectory()) {
    type = TRANSFERJOB_DOWNLOAD;
    srcfilelists[""] = new FileList(filelist->getUser(), filelist->getPath() / dstfile);
    updateLocalFileLists(path / dstfile);
  }
  else {
    type = TRANSFERJOB_DOWNLOAD_FILE;
  }
}

TransferJob::TransferJob(unsigned int id, const Path & path, std::string srcfile, const Pointer<SiteLogic> & sl, std::string dstfile, FileList * filelist) :
      src(NULL),
      dst(sl),
      srcfile(srcfile),
      dstfile(dstfile),
      localpath(path),
      dstlist(filelist),
      id(id)
{
  init();
  type = TRANSFERJOB_UPLOAD;
  if (FileSystem::directoryExistsReadable(path)) {
    locallist = global->getLocalStorage()->getLocalFileList(path);
    std::map<std::string, LocalFile>::const_iterator it = locallist->find(srcfile);
    if (it != locallist->end() && it->second.isDirectory()) {
      dstfilelists[""] = new FileList(filelist->getUser(), filelist->getPath() / dstfile);
      updateLocalFileLists(path / srcfile);
    }
    else {
      type = TRANSFERJOB_UPLOAD_FILE;
    }
  }
}

TransferJob::TransferJob(unsigned int id, const Pointer<SiteLogic> & slsrc, std::string srcfile, FileList * srcfilelist, const Pointer<SiteLogic> & sldst, std::string dstfile, FileList * dstfilelist) :
      src(slsrc),
      dst(sldst),
      srcfile(srcfile),
      dstfile(dstfile),
      srclist(srcfilelist),
      dstlist(dstfilelist),
      id(id)
{
  init();
  if (srclist->getFile(srcfile)->isDirectory()) {
    type = TRANSFERJOB_FXP;
    srcfilelists[""] = new FileList(srcfilelist->getUser(), srcfilelist->getPath() / srcfile);
    dstfilelists[""] = new FileList(dstfilelist->getUser(), dstfilelist->getPath() / dstfile);
  }
  else {
    type = TRANSFERJOB_FXP_FILE;
  }
}

TransferJob::~TransferJob() {
  if (!isDone()) {
    setDone();
  }
}

int TransferJob::classType() const {
  return COMMANDOWNER_TRANSFERJOB;
}

const Path & TransferJob::getLocalPath() const {
  return localpath;
}

FileList * TransferJob::getSrcFileList() const {
  return srclist;
}

FileList * TransferJob::getDstFileList() const {
  return dstlist;
}

Pointer<LocalFileList> TransferJob::getLocalFileList() const {
  return locallist;
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

std::map<std::string, Pointer<LocalFileList> >::const_iterator TransferJob::localFileListsBegin() const {
  return localfilelists.begin();
}

std::map<std::string, Pointer<LocalFileList> >::const_iterator TransferJob::localFileListsEnd() const {
  return localfilelists.end();
}

std::map<std::string, unsigned long long int>::const_iterator TransferJob::pendingTransfersBegin() const {
  return pendingtransfers.begin();
}

std::map<std::string, unsigned long long int>::const_iterator TransferJob::pendingTransfersEnd() const {
  return pendingtransfers.end();
}

std::list<Pointer<TransferStatus> >::const_iterator TransferJob::transfersBegin() const {
  return transfers.begin();
}

std::list<Pointer<TransferStatus> >::const_iterator TransferJob::transfersEnd() const {
  return transfers.end();
}

void TransferJob::init() {
  done = false;
  almostdone = false;
  listsrefreshed = false;
  initialized = false;
  aborted = false;
  slots = 1;
  srclisttarget = NULL;
  dstlisttarget = NULL;
  expectedfinalsize = 0;
  sizeprogress = 0;
  timeremaining = -1;
  timespentmillis = 0;
  timespentsecs = 0;
  progress = 0;
  speed = 0;
  filesprogress = 0;
  filestotal = 0;
  timestarted = util::ctimeLog();
  global->getTickPoke()->startPoke(this, "TransferJob", TRANSFERJOB_UPDATE_INTERVAL, 0);
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
  if (sl == src.get() && (type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP)) {
    for (std::map<std::string, FileList *>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
      std::map<FileList *, bool>::iterator it2 = filelistsrefreshed.find(it->second);
      FileListState state = it->second->getState();
      if ((state != FILELIST_LISTED && state != FILELIST_NONEXISTENT) || (it2 != filelistsrefreshed.end() && !it2->second)) {
        srclisttarget = it->second;
        return true;
      }
    }
  }
  else if (sl == dst.get() && (type == TRANSFERJOB_UPLOAD || type == TRANSFERJOB_FXP)) {
    for (std::map<std::string, FileList *>::iterator it = dstfilelists.begin(); it != dstfilelists.end(); it++) {
      std::map<FileList *, bool>::iterator it2 = filelistsrefreshed.find(it->second);
      FileListState state = it->second->getState();
      if ((state != FILELIST_LISTED && state != FILELIST_NONEXISTENT) || (it2 != filelistsrefreshed.end() && !it2->second)) {
        dstlisttarget = it->second;
        return true;
      }
    }
  }
  if (!filestotal) {
    countTotalFiles();
  }
  return false;
}

Pointer<LocalFileList> TransferJob::wantedLocalDstList(const std::string & subdir) {
  if (localfilelists.find(subdir) == localfilelists.end()) {
    localfilelists[subdir] = makePointer<LocalFileList>(localpath / dstfile / subdir);
  }
  return localfilelists.at(subdir);
}

FileList * TransferJob::getListTarget(SiteLogic * sl) const {
  if (sl == src.get()) {
    return srclisttarget;
  }
  else if (sl == dst.get()) {
    return dstlisttarget;
  }
  return NULL;
}

void TransferJob::fileListUpdated(SiteLogic * sl, FileList * fl) {
  std::map<FileList *, bool>::iterator it2 = filelistsrefreshed.find(fl);
  if (it2 == filelistsrefreshed.end()) {
    filelistsrefreshed[fl] = false;
    it2 = filelistsrefreshed.find(fl);
  }
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
    countTotalFiles();
  }
  Path subpath = fl->getPath() - (sl == src.get() ? srcfilelists[""]->getPath() : dstfilelists[""]->getPath());
  if (type == TRANSFERJOB_FXP || type == TRANSFERJOB_DOWNLOAD) {
    addSubDirectoryFileLists(srcfilelists, fl, subpath);
  }
  if (type == TRANSFERJOB_FXP || type == TRANSFERJOB_UPLOAD) {
    addSubDirectoryFileLists(dstfilelists, fl, subpath);
  }

}

FileList * TransferJob::findDstList(const std::string & sub) const {
  std::map<std::string, FileList *>::const_iterator it = dstfilelists.find(sub);
  if (it != dstfilelists.end()) {
    return it->second;
  }
  return NULL;
}

FileList * TransferJob::getFileListForFullPath(SiteLogic * sl, const Path & path) const {
  std::map<std::string, FileList *>::const_iterator it;
  if (sl == src.get()) {
    for (it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
      if (it->second->getPath() == path) {
        return it->second;
      }
    }
  }
  else if (sl == dst.get()) {
    for (it = dstfilelists.begin(); it != dstfilelists.end(); it++) {
      if (it->second->getPath() == path) {
        return it->second;
      }
    }
  }
  return NULL;
}

Pointer<LocalFileList> TransferJob::findLocalFileList(const std::string & subpath) const {
  std::map<std::string, Pointer<LocalFileList> >::const_iterator it = localfilelists.find(subpath);
  if (it != localfilelists.end()) {
    return it->second;
  }
  return Pointer<LocalFileList>();
}

void TransferJob::addSubDirectoryFileLists(std::map<std::string, FileList *> & filelists, FileList * filelist, const Path & subpath) {
  std::map<std::string, File *>::iterator it;
  for(it = filelist->begin(); it != filelist->end(); it++) {
    File * file = it->second;
    if (file->isDirectory()) {
      if (!global->getSkipList()->isAllowed(it->first, true, false)) {
        continue;
      }
      std::string filename = file->getName();
      if (filelists.find(filename) == filelists.end()) {
        filelists[filename] = new FileList(filelist->getUser(), filelists[""]->getPath() / subpath / filename);
      }
    }
  }
}

const Pointer<SiteLogic> & TransferJob::getSrc() const {
  return src;
}

const Pointer<SiteLogic> & TransferJob::getDst() const {
  return dst;
}

int TransferJob::maxSlots() const {
  return slots;
}

void TransferJob::setSlots(int slots) {
  this->slots = slots;
}

int TransferJob::maxPossibleSlots() const {
  switch (type) {
    case TRANSFERJOB_DOWNLOAD_FILE:
    case TRANSFERJOB_FXP_FILE:
    case TRANSFERJOB_UPLOAD_FILE:
      return 1;
    case TRANSFERJOB_UPLOAD:
      return dst->getSite()->getMaxUp();
    case TRANSFERJOB_FXP:
    case TRANSFERJOB_DOWNLOAD:
      return src->getSite()->getMaxDown();
  }
  return 0;
}

bool TransferJob::listsRefreshed() const {
  return listsrefreshed;
}

void TransferJob::refreshOrAlmostDone() {
  if (listsrefreshed || type == TRANSFERJOB_FXP_FILE ||
      type == TRANSFERJOB_DOWNLOAD_FILE || type == TRANSFERJOB_UPLOAD_FILE) {
    almostdone = true;
  }
  else {
    clearRefreshLists();
    for (std::map<std::string, FileList *>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
      filelistsrefreshed[it->second] = false;
    }
    for (std::map<std::string, FileList *>::iterator it = dstfilelists.begin(); it != dstfilelists.end(); it++) {
      filelistsrefreshed[it->second] = false;
    }
    if (type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_UPLOAD) {
      updateLocalFileLists(localpath / srcfile);
    }
  }

}

void TransferJob::clearRefreshLists() {
  filelistsrefreshed.clear();
  listsrefreshed = false;
}

void TransferJob::addPendingTransfer(const Path & name, unsigned long long int size) {
  pendingtransfers[name.toString()] = size;
}

void TransferJob::addTransfer(const Pointer<TransferStatus> & ts) {
  if (!!ts && ts->getState() != TRANSFERSTATUS_STATE_FAILED) {
    transfers.push_front(ts);
    Path subpathfile = findSubPath(ts) / ts->getFile();
    if (pendingtransfers.find(subpathfile.toString()) != pendingtransfers.end()) {
      pendingtransfers.erase(subpathfile.toString());
    }
  }
}

void TransferJob::targetExists(const Path & target) {
  existingtargets.insert(target.toString());
}

void TransferJob::tick(int message) {
  updateStatus();
  timespentmillis += TRANSFERJOB_UPDATE_INTERVAL;
  timespentsecs = timespentmillis / 1000;
}

void TransferJob::updateStatus() {
  unsigned long long int aggregatedsize = 0;
  unsigned long long int aggregatedsizetransferred = 0;
  int aggregatedfilescomplete = 0;
  bool ongoingtransfers = false;
  for (std::list<Pointer<TransferStatus> >::const_iterator it = transfersBegin(); it != transfersEnd(); it++) {
    if (pendingtransfers.find((*it)->getFile()) != pendingtransfers.end()) {
      pendingtransfers.erase((*it)->getFile());
    }
    int state = (*it)->getState();
    if (state != TRANSFERSTATUS_STATE_FAILED) {
      aggregatedsize += (*it)->sourceSize();
    }
    if (state == TRANSFERSTATUS_STATE_IN_PROGRESS || state == TRANSFERSTATUS_STATE_SUCCESSFUL) {
      aggregatedsizetransferred += (*it)->targetSize();
    }
    if (state == TRANSFERSTATUS_STATE_IN_PROGRESS) {
      ongoingtransfers = true;
    }
    if (state == TRANSFERSTATUS_STATE_SUCCESSFUL) {
      Path subpathfile = findSubPath(*it) / (*it)->getFile();
      if (existingtargets.find(subpathfile.toString()) == existingtargets.end()) {
        aggregatedfilescomplete++;
      }
    }
  }
  for (std::map<std::string, unsigned long long int>::const_iterator it = pendingTransfersBegin(); it != pendingTransfersEnd(); it++) {
    aggregatedsize += it->second;
  }
  expectedfinalsize = aggregatedsize;
  sizeprogress = aggregatedsizetransferred;
  if (expectedfinalsize) {
    progress = (100 * sizeprogress) / expectedfinalsize;
  }
  if (timespentmillis) {
    speed = sizeprogress / timespentmillis;
  }
  if (speed) {
    timeremaining = (expectedfinalsize - sizeprogress) / (speed * 1024);
  }
  filesprogress = existingtargets.size() + aggregatedfilescomplete;
  if (almostdone && !ongoingtransfers && filesprogress >= filestotal) {
    setDone();
  }
}

int TransferJob::getProgress() const {
  return progress;
}

int TransferJob::timeSpent() const {
  return timespentsecs;
}

int TransferJob::timeRemaining() const {
  return timeremaining;
}

unsigned long long int TransferJob::sizeProgress() const {
  return sizeprogress;
}

unsigned long long int TransferJob::totalSize() const {
  return expectedfinalsize;
}

unsigned int TransferJob::getSpeed() const {
  return speed;
}

std::string TransferJob::timeStarted() const {
  return timestarted;
}

std::string TransferJob::typeString() const {
  std::string type;
  switch (this->type) {
    case TRANSFERJOB_DOWNLOAD:
    case TRANSFERJOB_DOWNLOAD_FILE:
      type = "Download";
      break;
    case TRANSFERJOB_UPLOAD:
    case TRANSFERJOB_UPLOAD_FILE:
      type = "Upload";
      break;
    case TRANSFERJOB_FXP:
    case TRANSFERJOB_FXP_FILE:
      type = "FXP";
      break;
  }
  return type;
}

int TransferJob::filesProgress() const {
  return filesprogress;
}

int TransferJob::filesTotal() const {
  return filestotal;
}

void TransferJob::countTotalFiles() {
  switch (type) {
    case TRANSFERJOB_DOWNLOAD_FILE:
    case TRANSFERJOB_UPLOAD_FILE:
    case TRANSFERJOB_FXP_FILE:
      filestotal = 1;
      break;
    case TRANSFERJOB_DOWNLOAD:
    case TRANSFERJOB_FXP:
    {
      int files = 0;
      for (std::map<std::string, FileList *>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
        files += it->second->getNumUploadedFiles();
      }
      filestotal = files;
      break;
    }
    case TRANSFERJOB_UPLOAD:
    {
      int files = 0;
      std::map<std::string, Pointer<LocalFileList> >::const_iterator it;
      for (it = localfilelists.begin(); it != localfilelists.end(); it++) {
        files += it->second->sizeFiles();
      }
      filestotal = files;
      break;
    }
  }
}

Path TransferJob::findSubPath(const Pointer<TransferStatus> & ts) const {
  Path path = ts->getSourcePath();
  switch (type) {
    case TRANSFERJOB_DOWNLOAD:
    case TRANSFERJOB_FXP:
      for (std::map<std::string, FileList *>::const_iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
        if (it->second->getPath() == path) {
          return it->first;
        }
      }
      break;
    case TRANSFERJOB_UPLOAD: {
      Path relpath = localpath / srcfile;
      if (path != relpath && path.contains(relpath) != std::string::npos) {
        return relpath - path;
      }
      break;
    }
  }
  return "";
}

bool TransferJob::isInitialized() const {
  return initialized;
}

void TransferJob::setInitialized() {
  initialized = true;
}

void TransferJob::abort() {
  aborted = true;
  setDone();
}

void TransferJob::clearExisting() {
  existingtargets.clear();
}

bool TransferJob::isAborted() const {
  return aborted;
}

unsigned int TransferJob::getId() const {
  return id;
}

void TransferJob::setDone() {
  global->getTickPoke()->stopPoke(this, 0);
  done = true;
  timeremaining = 0;
}

void TransferJob::updateLocalFileLists(const Path & basepath) {
  localfilelists.clear();
  Pointer<LocalFileList> base = global->getLocalStorage()->getLocalFileList(basepath);
  if (!!base) {
    localfilelists[""] = base;
    std::map<std::string, LocalFile>::const_iterator it;
    for (it = base->begin(); it != base->end(); it++) {
      if (it->second.isDirectory()) {
        localfilelists[it->first] = global->getLocalStorage()->getLocalFileList(base->getPath() / it->first);
        if (type == TRANSFERJOB_DOWNLOAD && srcfilelists.find(it->first) == srcfilelists.end()) {
          srcfilelists[it->first] = new FileList(srclist->getUser(), srclist->getPath() / srcfile / it->first);
        }
        else if (type == TRANSFERJOB_UPLOAD && dstfilelists.find(it->first) == dstfilelists.end()) {
          dstfilelists[it->first] = new FileList(dstlist->getUser(), dstlist->getPath() / dstfile / it->first);
        }
      }
    }
  }
}
