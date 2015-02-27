#include "transferjob.h"

#include "filelist.h"
#include "file.h"
#include "site.h"
#include "globalcontext.h"
#include "skiplist.h"
#include "transferstatus.h"
#include "tickpoke.h"
#include "sitelogic.h"
#include "util.h"

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
  if ((type == TRANSFERJOB_DOWNLOAD || type == TRANSFERJOB_FXP) && sl == src) {
    for (std::map<std::string, FileList *>::iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
      std::map<FileList *, bool>::iterator it2 = filelistsrefreshed.find(it->second);
      if (!it->second->isFilled() || (it2 != filelistsrefreshed.end() && !it2->second)) {
        srclisttarget = it->second;
        return true;
      }
    }
  }
  if (!filestotal) {
    countTotalFiles();
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
      countTotalFiles();
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

int TransferJob::maxPossibleSlots() const {
  switch (type) {
    case TRANSFERJOB_DOWNLOAD_FILE:
    case TRANSFERJOB_FXP_FILE:
    case TRANSFERJOB_UPLOAD_FILE:
      return 1;
    case TRANSFERJOB_FXP:
    {
      int max1 = src->getSite()->getMaxDown();
      int max2 = dst->getSite()->getMaxUp();
      return max2 < max1 ? max2 : max1;
    }
    case TRANSFERJOB_UPLOAD:
      return dst->getSite()->getMaxUp();
    case TRANSFERJOB_DOWNLOAD:
      return src->getSite()->getMaxDown();
  }
  return 0;
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

void TransferJob::setAlmostDone() {
  almostdone = true;
}

void TransferJob::clearRefreshLists() {
  filelistsrefreshed.clear();
  listsrefreshed = false;
}

void TransferJob::addPendingTransfer(std::string name, unsigned long long int size) {
  pendingtransfers[name] = size;
}

void TransferJob::addTransfer(Pointer<TransferStatus> ts) {
  if (!!ts && ts->getState() != TRANSFERSTATUS_STATE_FAILED) {
    transfers.push_front(ts);
    std::string subpathfile = findSubPath(ts) + ts->getFile();
    if (pendingtransfers.find(subpathfile) != pendingtransfers.end()) {
      pendingtransfers.erase(subpathfile);
    }
  }
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
    aggregatedsize += (*it)->sourceSize();
    int state = (*it)->getState();
    if (state == TRANSFERSTATUS_STATE_IN_PROGRESS || state == TRANSFERSTATUS_STATE_SUCCESSFUL) {
      aggregatedsizetransferred += (*it)->targetSize();
    }
    if (state == TRANSFERSTATUS_STATE_IN_PROGRESS) {
      ongoingtransfers = true;
    }
    if (state == TRANSFERSTATUS_STATE_SUCCESSFUL) {
      aggregatedfilescomplete++;
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
  filesprogress = aggregatedfilescomplete;
  if (almostdone && !ongoingtransfers && filesprogress >= filestotal) {
    done = true;
    global->getTickPoke()->stopPoke(this, 0);
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
      break;
  }
}

std::string TransferJob::findSubPath(Pointer<TransferStatus> ts) const {
  std::string path = ts->getSourcePath();
  switch (type) {
    case TRANSFERJOB_DOWNLOAD:
    case TRANSFERJOB_FXP:
      for (std::map<std::string, FileList *>::const_iterator it = srcfilelists.begin(); it != srcfilelists.end(); it++) {
        if (it->second->getPath() == path) {
          return it->first.length() ? it->first + "/" : "";
        }
      }
      break;
    case TRANSFERJOB_UPLOAD:
      if (path != srcpath && path.find(srcpath) != std::string::npos) {
        return path.substr(srcpath.length() + 1) + "/";
      }
      break;
  }
  return "";
}

bool TransferJob::isInitialized() const {
  return initialized;
}

void TransferJob::setInitialized() {
  initialized = true;
}
