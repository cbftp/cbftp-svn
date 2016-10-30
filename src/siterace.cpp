#include "siterace.h"

#include "race.h"
#include "filelist.h"
#include "file.h"
#include "globalcontext.h"
#include "skiplist.h"
#include "util.h"
#include "timereference.h"

SiteRace::SiteRace(Pointer<Race> race, const std::string & sitename, const Path & section, const std::string & release, const std::string & username) :
  race(race),
  section(section),
  release(release),
  path(section / release),
  group(util::getGroupNameFromRelease(release)),
  username(username),
  sitename(sitename),
  done(false),
  maxfilesize(0),
  totalfilesize(0),
  numuploadedfiles(0),
  profile(race->getProfile())
{
  recentlyvisited.push_back("");
  filelists[""] = new FileList(username, path);
}

SiteRace::~SiteRace() {
  for (std::map<std::string, FileList *>::iterator it = filelists.begin(); it != filelists.end(); it++) {
    delete it->second;
  }
}

int SiteRace::classType() const {
  return COMMANDOWNER_SITERACE;
}

std::string SiteRace::getSiteName() const {
  return sitename;
}

const Path & SiteRace::getSection() const {
  return section;
}

std::string SiteRace::getRelease() const {
  return release;
}

std::string SiteRace::getGroup() const {
  return group;
}

const Path & SiteRace::getPath() const {
  return path;
}

unsigned int SiteRace::getId() const {
  return race->getId();
}

bool SiteRace::addSubDirectory(const std::string & subpath) {
  return addSubDirectory(subpath, false);
}

bool SiteRace::addSubDirectory(const std::string & subpath, bool knownexists) {
  if (!global->getSkipList()->isAllowed(subpath, true)) {
    return false;
  }
  if (getFileListForPath(subpath) != NULL) {
    return true;
  }
  FileList * subdir;
  if (knownexists) {
    subdir = new FileList(username, path / subpath, FILELIST_EXISTS);
  }
  else {
    subdir = new FileList(username, path / subpath);
  }
  filelists[subpath] = subdir;
  recentlyvisited.push_front(subpath);
  if (knownexists) {
    race->reportNewSubDir(this, subpath);
  }
  return true;
}

std::string SiteRace::getSubPath(FileList * filelist) const {
  std::map<std::string, FileList *>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == filelist) {
      return it->first;
    }
  }
  // internal error
  return "";
}

std::string SiteRace::getRelevantSubPath() {
  while (recentlyvisited.size()) {
    std::string leastrecentlyvisited = recentlyvisited.front();
    recentlyvisited.pop_front();
    if (!isSubPathComplete(leastrecentlyvisited)) {
      recentlyvisited.push_back(leastrecentlyvisited);
      return leastrecentlyvisited;
    }
  }
  return "";
}

FileList * SiteRace::getFileListForPath(const std::string & subpath) const {
  std::map<std::string, FileList *>::const_iterator it = filelists.find(subpath);
  if (it != filelists.end()) {
    return it->second;
  }
  return NULL;
}

FileList * SiteRace::getFileListForFullPath(SiteLogic *, const Path & path) const {
  std::map<std::string, FileList *>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->getPath() == path) {
      return it->second;
    }
  }
  return NULL;
}

std::string SiteRace::getSubPathForFileList(FileList * fl) const {
  std::map<std::string, FileList *>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == fl) {
      return it->first;
    }
  }
  return "";
}

std::map<std::string, FileList *>::const_iterator SiteRace::fileListsBegin() const {
  return filelists.begin();
}

std::map<std::string, FileList *>::const_iterator SiteRace::fileListsEnd() const {
  return filelists.end();
}

void SiteRace::fileListUpdated(FileList *) {
  updateNumFilesUploaded();
  addNewDirectories();
}

void SiteRace::updateNumFilesUploaded() {
  std::map<std::string, FileList *>::iterator it;
  unsigned int sum = 0;
  unsigned long long int maxsize = 0;
  unsigned long long int maxsizewithfiles = 0;
  unsigned long long int aggregatedfilesize = 0;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    FileList * fl = it->second;
    sum += fl->getNumUploadedFiles();
    aggregatedfilesize += fl->getTotalFileSize();
    if (fl->hasSFV()) {
      race->reportSFV(this, it->first);
    }
    unsigned long long int max = fl->getMaxFileSize();
    if (max > maxsize) {
      maxsize = max;
      if (fl->getSize() >= 5) {
        maxsizewithfiles = max;
      }
    }
  }
  if (maxsizewithfiles > 0) {
    this->maxfilesize = maxsizewithfiles;
  }
  else {
    this->maxfilesize = maxsize;
  }
  numuploadedfiles = sum;
  totalfilesize = aggregatedfilesize;
  race->updateSiteProgress(sum);
}

void SiteRace::addNewDirectories() {
  FileList * filelist = getFileListForPath("");
  std::map<std::string, File *>::iterator it;
  for(it = filelist->begin(); it != filelist->end(); it++) {
    File * file = it->second;
    if (file->isDirectory()) {
      if (!global->getSkipList()->isAllowed(it->first, true)) {
        continue;
      }
      FileList * fl;
      if ((fl = getFileListForPath(file->getName())) == NULL) {
        addSubDirectory(file->getName(), true);
      }
      else if (fl->getState() == FILELIST_UNKNOWN || fl->getState() == FILELIST_NONEXISTENT) {
        fl->setExists();
        race->reportNewSubDir(this, it->first);
      }
    }
  }
}

unsigned int SiteRace::getNumUploadedFiles() const {
  return numuploadedfiles;
}

bool SiteRace::sizeEstimated(FileList * fl) const {
  std::list<FileList *>::const_iterator it;
  for (it = sizeestimated.begin(); it != sizeestimated.end(); it++) {
    if (*it == fl) {
      return true;
    }
  }
  return false;
}

unsigned long long int SiteRace::getMaxFileSize() const {
  return maxfilesize;
}

unsigned long long int SiteRace::getTotalFileSize() const {
  return totalfilesize;
}

bool SiteRace::isDone() const {
  return done;
}

bool SiteRace::isGlobalDone() const {
  return race->isDone();
}

void SiteRace::complete(bool report) {
  done = true;
  if (report) {
    race->reportDone(this);
  }
}

void SiteRace::abort() {
  done = true;
}

void SiteRace::reset() {
  done = false;
  filelists.clear(); // memory leak, use Pointer<FileList> instead
  filelists[""] = new FileList(username, path);
  recentlyvisited.clear();
  recentlyvisited.push_back("");
  sfvobservestarts.clear();
  observestarts.clear();
  sizeestimated.clear();
  completesubdirs.clear();
  maxfilesize = 0;
  totalfilesize = 0;
  numuploadedfiles = 0;
}

void SiteRace::subPathComplete(FileList * fl) {
  std::string subpath = getSubPathForFileList(fl);
  if (isSubPathComplete(subpath)) {
    return;
  }
  completesubdirs.push_back(subpath);
}

bool SiteRace::isSubPathComplete(const std::string & subpath) const {
  std::list<std::string>::const_iterator it;
  for (it = completesubdirs.begin(); it != completesubdirs.end(); it++) {
    if (*it == subpath) {
      return true;
    }
  }
  return false;
}

bool SiteRace::isSubPathComplete(FileList * fl) const {
  std::string subpath = getSubPathForFileList(fl);
  return isSubPathComplete(subpath);
}

Pointer<Race> SiteRace::getRace() const {
  return race;
}

int SiteRace::getProfile() const {
  return profile;
}

void SiteRace::reportSize(FileList * fl, std::list<std::string> * uniques, bool final) {
  if (!sizeEstimated(fl)) {
    std::map<std::string, FileList *>::iterator it;
    for (it = filelists.begin(); it != filelists.end(); it++) {
      if (it->second == fl) {
        race->reportSize(this, fl, it->first, uniques, final);
        if (final) {
          sizeestimated.push_back(fl);
        }
        return;
      }
    }
  }
}

int SiteRace::getObservedTime(FileList * fl) {
  std::map<FileList *, unsigned long long int>::iterator it;
  for (it = observestarts.begin(); it != observestarts.end(); it++) {
    if (it->first == fl) {
      return global->getTimeReference()->timePassedSince(it->second);
    }
  }
  if (fl->getSize() > 0) {
    observestarts[fl] = global->getTimeReference()->timeReference();
  }
  return 0;
}

int SiteRace::getSFVObservedTime(FileList * fl) {
  std::map<FileList *, unsigned long long int>::iterator it;
  for (it = sfvobservestarts.begin(); it != sfvobservestarts.end(); it++) {
    if (it->first == fl) {
      return global->getTimeReference()->timePassedSince(it->second);
    }
  }
  sfvobservestarts[fl] = global->getTimeReference()->timeReference();
  return 0;
}

bool SiteRace::hasBeenUpdatedSinceLastCheck() {
  bool changed = false;
  std::map<std::string, FileList *>::iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->listChanged()) {
      changed = true;
    }
    it->second->resetListChanged();
  }
  return changed;
}
