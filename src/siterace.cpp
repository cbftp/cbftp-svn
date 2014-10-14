#include "siterace.h"

#include "filelist.h"
#include "file.h"
#include "race.h"
#include "globalcontext.h"
#include "skiplist.h"

extern GlobalContext * global;

SiteRace::SiteRace(Race * race, std::string section, std::string release, std::string username) {
  this->race = race;
  this->section = section;
  this->release = release;
  this->username = username;
  size_t splitpos = release.rfind("-");
  if (splitpos != std::string::npos) {
    this->group = release.substr(splitpos + 1);
  }
  else {
    this->group = "";
  }
  recentlyvisited.push_back("");
  path = section.append("/").append(release);
  FileList * rootdir = new FileList(username, path);
  filelists[""] = rootdir;
  done = false;
  maxfilesize = 0;
}

SiteRace::~SiteRace() {
  for (std::map<std::string, FileList *>::iterator it = filelists.begin(); it != filelists.end(); it++) {
    delete it->second;
  }
}

std::string SiteRace::getSection() const {
  return section;
}

std::string SiteRace::getRelease() const {
  return release;
}

std::string SiteRace::getGroup() const {
  return group;
}

std::string SiteRace::getPath() const {
  return path;
}

void SiteRace::addSubDirectory(std::string subpath) {
  if (!global->getSkipList()->isAllowed(subpath, true)) return;
  if (getFileListForPath(subpath) != NULL) {
    return;
  }
  FileList * subdir = new FileList(username, path + "/" + subpath);
  filelists[subpath] = subdir;
  recentlyvisited.push_front(subpath);
  race->reportNewSubDir(this, subpath);
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
  std::string leastrecentlyvisited = recentlyvisited.front();
  while (isSubPathComplete(leastrecentlyvisited) && recentlyvisited.size() > 0) {
    recentlyvisited.pop_front();
    leastrecentlyvisited = recentlyvisited.front();
  }
  if (recentlyvisited.size() == 0) {
    return "";
  }
  recentlyvisited.push_back(leastrecentlyvisited);
  recentlyvisited.pop_front();
  return leastrecentlyvisited;
}

FileList * SiteRace::getFileListForPath(std::string subpath) const {
  std::map<std::string, FileList *>::const_iterator it = filelists.find(subpath);
  if (it != filelists.end()) {
    return it->second;
  }
  return NULL;
}

FileList * SiteRace::getFileListForFullPath(std::string path) const {
  std::map<std::string, FileList *>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->getPath() == path) {
      return it->second;
    }
  }
  //std::cout << "None found LOL" << std::endl;
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

void SiteRace::updateNumFilesUploaded() {
  std::map<std::string, FileList *>::iterator it;
  int sum = 0;
  unsigned long long int maxsize = 0;
  unsigned long long int maxsizewithfiles = 0;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    sum += it->second->getSize();
    if (it->second->hasSFV()) {
      race->reportSFV(this, it->first);
    }
    unsigned long long int max = it->second->getMaxFileSize();
    if (max > maxsize) {
      maxsize = max;
      if (it->second->getSize() >= 5) {
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
  race->updateSiteProgress(sum);
}

void SiteRace::addNewDirectories() {
  FileList * filelist = getFileListForPath("");
  std::map<std::string, File *>::iterator it;
  for(it = filelist->begin(); it != filelist->end(); it++) {
    if (!global->getSkipList()->isAllowed(it->first, true)) {
      continue;
    }
    File * file = it->second;
    if (file->isDirectory()) {
      if (getFileListForPath(file->getName()) == NULL) {
        addSubDirectory(file->getName());
      }
    }
  }
}

int SiteRace::getNumUploadedFiles() const {
  std::map<std::string, FileList *>::const_iterator it;
  int sum = 0;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    sum += it->second->getNumUploadedFiles();
  }
  return sum;
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

bool SiteRace::isDone() const {
  return done;
}

void SiteRace::complete() {
  done = true;
  race->reportDone(this);
}

void SiteRace::abort() {
  done = true;
}

void SiteRace::subPathComplete(FileList * fl) {
  std::string subpath = getSubPathForFileList(fl);
  if (isSubPathComplete(subpath)) {
    return;
  }
  completesubdirs.push_back(subpath);
}

bool SiteRace::isSubPathComplete(std::string subpath) const {
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

Race * SiteRace::getRace() const {
  return race;
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
  std::map<FileList *, int>::iterator it;
  for (it = observestarts.begin(); it != observestarts.end(); it++) {
    if (it->first == fl) {
      return global->ctimeMSec() - it->second;
    }
  }
  observestarts[fl] = global->ctimeMSec();
  return 0;
}

int SiteRace::getSFVObservedTime(FileList * fl) {
  std::map<FileList *, int>::iterator it;
  for (it = sfvobservestarts.begin(); it != sfvobservestarts.end(); it++) {
    if (it->first == fl) {
      return global->ctimeMSec() - it->second;
    }
  }
  sfvobservestarts[fl] = global->ctimeMSec();
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

void SiteRace::addVisitedPath(std::string path) {
  visitedpaths[path] = true;
}

bool SiteRace::pathVisited(std::string path) const {
  return visitedpaths.find(path) != visitedpaths.end();
}
