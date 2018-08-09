#include "siterace.h"

#include "race.h"
#include "filelist.h"
#include "file.h"
#include "globalcontext.h"
#include "skiplist.h"
#include "util.h"
#include "timereference.h"

SiteRace::SiteRace(std::shared_ptr<Race> race, const std::string & sitename, const Path & section, const std::string & release, const std::string & username, const SkipList & skiplist) :
  race(race),
  section(section),
  release(release),
  path(section / release),
  group(util::getGroupNameFromRelease(release)),
  username(username),
  sitename(sitename),
  done(false),
  aborted(false),
  donebeforeabort(false),
  maxfilesize(0),
  totalfilesize(0),
  numuploadedfiles(0),
  profile(race->getProfile()),
  skiplist(skiplist),
  sizedown(0),
  filesdown(0),
  speeddown(1),
  sizeup(0),
  filesup(0),
  speedup(1)
{
  recentlyvisited.push_back("");
  filelists[""] = new FileList(username, path);
}

SiteRace::~SiteRace() {
  for (std::unordered_map<std::string, FileList *>::iterator it = filelists.begin(); it != filelists.end(); it++) {
    delete it->second;
  }
}

std::string SiteRace::getName() const {
  return release;
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
  SkipListMatch match = skiplist.check(subpath, true);
  if (match.action == SKIPLIST_DENY) {
    return false;
  }
  if (getFileListForPath(subpath) != NULL) {
    return true;
  }
  if (match.action == SKIPLIST_UNIQUE && filelists[""]->containsPatternBefore(match.matchpattern, true, subpath)) {
    return false;
  }
  FileList * subdir;
  if (knownexists) {
    subdir = new FileList(username, path / subpath, FILELIST_EXISTS);
  }
  else {
    subdir = new FileList(username, path / subpath);
  }
  filelists[subpath] = subdir;
  recentlyvisited.push_back(subpath);
  if (knownexists) {
    race->reportNewSubDir(this, subpath);
  }
  return true;
}

std::string SiteRace::getSubPath(FileList * filelist) const {
  std::unordered_map<std::string, FileList *>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == filelist) {
      return it->first;
    }
  }
  // internal error
  return "";
}

std::string SiteRace::getRelevantSubPath() {
  for (unsigned int i = 0; i < recentlyvisited.size() && recentlyvisited.size(); i++) {
    std::string leastrecentlyvisited = recentlyvisited.front();
    recentlyvisited.pop_front();
    if (!isSubPathComplete(leastrecentlyvisited)) {
      recentlyvisited.push_back(leastrecentlyvisited);
      if (filelists[leastrecentlyvisited]->getState() != FILELIST_NONEXISTENT &&
          filelists[leastrecentlyvisited]->getState() != FILELIST_FAILED)
      {
        return leastrecentlyvisited;
      }
    }
  }
  return "";
}

bool SiteRace::anyFileListNotNonExistent() const {
  std::unordered_map<std::string, FileList *>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->getState() != FILELIST_NONEXISTENT) {
      return true;
    }
  }
  return false;
}

FileList * SiteRace::getFileListForPath(const std::string & subpath) const {
  std::unordered_map<std::string, FileList *>::const_iterator it = filelists.find(subpath);
  if (it != filelists.end()) {
    return it->second;
  }
  return NULL;
}

FileList * SiteRace::getFileListForFullPath(SiteLogic *, const Path & path) const {
  std::unordered_map<std::string, FileList *>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->getPath() == path) {
      return it->second;
    }
  }
  return NULL;
}

std::string SiteRace::getSubPathForFileList(FileList * fl) const {
  std::unordered_map<std::string, FileList *>::const_iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == fl) {
      return it->first;
    }
  }
  return "";
}

std::unordered_map<std::string, FileList *>::const_iterator SiteRace::fileListsBegin() const {
  return filelists.begin();
}

std::unordered_map<std::string, FileList *>::const_iterator SiteRace::fileListsEnd() const {
  return filelists.end();
}

void SiteRace::fileListUpdated(SiteLogic *, FileList * fl) {
  updateNumFilesUploaded();
  addNewDirectories();
  markNonExistent(fl);
}

void SiteRace::updateNumFilesUploaded() {
  std::unordered_map<std::string, FileList *>::iterator it;
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
  std::unordered_map<std::string, File *>::iterator it;
  for(it = filelist->begin(); it != filelist->end(); it++) {
    File * file = it->second;
    if (file->isDirectory()) {
      SkipListMatch match = skiplist.check(it->first, true);
      if (match.action == SKIPLIST_DENY || (match.action == SKIPLIST_UNIQUE && filelist->containsPatternBefore(match.matchpattern, true, it->first))) {
        continue;
      }
      FileList * fl;
      if ((fl = getFileListForPath(file->getName())) == NULL) {
        addSubDirectory(file->getName(), true);
      }
      else if (fl->getState() == FILELIST_UNKNOWN || fl->getState() == FILELIST_NONEXISTENT ||
               fl->getState() == FILELIST_FAILED)
      {
        fl->setExists();
        race->reportNewSubDir(this, it->first);
      }
    }
  }
}

void SiteRace::markNonExistent(FileList * fl) {
  std::unordered_map<std::string, FileList *>::iterator it;
  bool found = false;
  for (it = filelists.begin() ;it != filelists.end(); it++) {
    if (fl == it->second) {
      found = true;
    }
  }
  if (!found) {
    return;
  }
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second != fl && it->second->getState() == FILELIST_UNKNOWN &&
        it->second->getPath().contains(fl->getPath()))
    {
      if (fl->getState() == FILELIST_NONEXISTENT ||
          !fl->contains((it->second->getPath() - fl->getPath()).level(1).toString()))
      {
        it->second->setNonExistent();
      }
    }
  }
}

unsigned int SiteRace::getNumUploadedFiles() const {
  return numuploadedfiles;
}

bool SiteRace::sizeEstimated(FileList * fl) const {
  return sizeestimated.find(fl) != sizeestimated.end();
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

bool SiteRace::isAborted() const {
  return aborted;
}

bool SiteRace::doneBeforeAbort() const {
  return donebeforeabort;
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
  if (!aborted) {
    donebeforeabort = done;
    done = true;
    aborted = true;
  }
}

void SiteRace::softReset() {
  recentlyvisited.clear();
  for (std::unordered_map<std::string, FileList *>::const_iterator it = filelists.begin(); it != filelists.end(); ++it) {
    recentlyvisited.push_back(it->first);
  }
  filelists.clear();
  for (std::list<std::string>::const_iterator it = recentlyvisited.begin(); it != recentlyvisited.end(); ++it) {
    filelists[*it] = new FileList(username, path / *it, FILELIST_FAILED);
  }
  reset();

}

void SiteRace::hardReset() {
  filelists.clear(); // memory leak, use std::shared_ptr<FileList> instead
  filelists[""] = new FileList(username, path);
  recentlyvisited.clear();
  recentlyvisited.push_back("");
  reset();
}

void SiteRace::reset() {
  done = false;
  aborted = false;
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

std::shared_ptr<Race> SiteRace::getRace() const {
  return race;
}

int SiteRace::getProfile() const {
  return profile;
}

void SiteRace::reportSize(FileList * fl, const std::unordered_set<std::string> & uniques, bool final) {
  std::unordered_map<std::string, FileList *>::iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == fl) {
      race->reportSize(this, fl, it->first, uniques, final);
      if (final) {
        sizeestimated.insert(fl);
      }
      return;
    }
  }
}

int SiteRace::getObservedTime(FileList * fl) {
  std::unordered_map<FileList *, unsigned long long int>::iterator it;
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
  std::unordered_map<FileList *, unsigned long long int>::iterator it;
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
  std::unordered_map<std::string, FileList *>::iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->listChanged() || it->second->hasFilesUploading()) {
      changed = true;
    }
    it->second->resetListChanged();
  }
  return changed;
}

void SiteRace::addTransferStatsFile(StatsDirection direction, const std::string & other, unsigned long long int size, unsigned int speed) {
  if (direction == STATS_DOWN) {
    race->addTransferStatsFile(size);
    if (sitessizedown.find(other) == sitessizedown.end()) {
      sitessizedown[other] = 0;
      sitesfilesdown[other] = 0;
      sitesspeeddown[other] = 1;
    }
    if (speed && size && size > speed) {
      sitesspeeddown[other] = (sitessizedown[other] + size) / (sitessizedown[other] / sitesspeeddown[other] + size / speed);
      if (!sitesspeeddown[other]) {
        ++sitesspeeddown[other];
      }
      speeddown = (sizedown + size) / (sizedown / speeddown + size / speed);
      if (!speeddown) {
        ++speeddown;
      }
    }
    sitessizedown[other] += size;
    sitesfilesdown[other] += 1;
    sizedown += size;
    ++filesdown;
  }
  else {
    if (sitessizeup.find(other) == sitessizeup.end()) {
      sitessizeup[other] = 0;
      sitesfilesup[other] = 0;
      sitesspeedup[other] = 1;
    }
    if (speed && size && size > speed) {
      sitesspeedup[other] = (sitessizeup[other] + size) / (sitessizeup[other] / sitesspeedup[other] + size / speed);
      if (!sitesspeedup[other]) {
        ++sitesspeedup[other];
      }
      speedup = (sizeup + size) / (sizeup / speedup + size / speed);
      if (!speedup) {
        ++speedup;
      }
    }
    sitessizeup[other] += size;
    sitesfilesup[other] += 1;
    sizeup += size;
    ++filesup;
  }
}

unsigned long long int SiteRace::getSizeDown() const {
  return sizedown;
}

unsigned int SiteRace::getFilesDown() const {
  return filesdown;
}

unsigned int SiteRace::getSpeedDown() const {
  return speeddown;
}

unsigned long long int SiteRace::getSizeUp() const {
  return sizeup;
}

unsigned int SiteRace::getFilesUp() const {
  return filesup;
}

unsigned int SiteRace::getSpeedUp() const {
  return speedup;
}

std::unordered_map<std::string, unsigned long long int>::const_iterator SiteRace::sizeUpBegin() const {
  return sitessizeup.begin();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::filesUpBegin() const {
  return sitesfilesup.begin();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::speedUpBegin() const {
  return sitesspeedup.begin();
}

std::unordered_map<std::string, unsigned long long int>::const_iterator SiteRace::sizeDownBegin() const {
  return sitessizedown.begin();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::filesDownBegin() const {
  return sitesfilesdown.begin();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::speedDownBegin() const {
  return sitesspeeddown.begin();
}

std::unordered_map<std::string, unsigned long long int>::const_iterator SiteRace::sizeUpEnd() const {
  return sitessizeup.end();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::filesUpEnd() const {
  return sitesfilesup.end();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::speedUpEnd() const {
  return sitesspeedup.end();
}

std::unordered_map<std::string, unsigned long long int>::const_iterator SiteRace::sizeDownEnd() const {
  return sitessizedown.end();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::filesDownEnd() const {
  return sitesfilesdown.end();
}

std::unordered_map<std::string, unsigned int>::const_iterator SiteRace::speedDownEnd() const {
  return sitesspeeddown.end();
}
