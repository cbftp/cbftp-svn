#include "siterace.h"

SiteRace::SiteRace(Race * race, std::string section, std::string release, std::string username) {
  this->race = race;
  this->section = section;
  this->release = release;
  this->username = username;
  sizeestimated = false;
  recentlyvisited.push_back("");
  path = section.append("/").append(release);
  FileList * rootdir = new FileList(username, path);
  filelists[""] = rootdir;
  recentlyvisited.push_back("");
  done = false;
}

std::string SiteRace::getSection() {
  return section;
}

std::string SiteRace::getRelease() {
  return release;
}

std::string SiteRace::getPath() {
  return path;
}

void SiteRace::addSubDirectory(std::string subpath) {
  if (getFileListForPath(subpath) != NULL) {
    return;
  }
  FileList * subdir = new FileList(username, path + "/" + subpath);
  filelists[subpath] = subdir;
  recentlyvisited.push_front(subpath);
}

std::string SiteRace::getSubPath(FileList * filelist) {
  std::map<std::string, FileList *>::iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second == filelist) {
      return it->first;
    }
  }
  // internal error
  return "";
}

std::string SiteRace::getLeastRecentlyVisitedSubPath() {
  std::string leastrecentlyvisited = recentlyvisited.front();
  recentlyvisited.push_back(leastrecentlyvisited);
  recentlyvisited.pop_front();
  if (leastrecentlyvisited.length() > 0) {
    leastrecentlyvisited = "/" + leastrecentlyvisited;
  }
  return leastrecentlyvisited;
}

FileList * SiteRace::getFileListForPath(std::string subpath) {
  std::map<std::string, FileList *>::iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->first == subpath) {
      return it->second;
    }
  }
  return NULL;
}

FileList * SiteRace::getFileListForFullPath(std::string path) {
  std::map<std::string, FileList *>::iterator it;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    if (it->second->getPath() == path) {
      return it->second;
    }
  }
  std::cout << "None found LOL" << std::endl;
  return NULL;
}

std::map<std::string, FileList *>::iterator SiteRace::fileListsBegin() {
  return filelists.begin();
}

std::map<std::string, FileList *>::iterator SiteRace::fileListsEnd() {
  return filelists.end();
}

void SiteRace::updateNumFilesUploaded() {
  std::map<std::string, FileList *>::iterator it;
  int sum = 0;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    sum += it->second->getSize();
  }
  race->updateSiteProgress(sum);
}

void SiteRace::addNewDirectories() {
  FileList * filelist = getFileListForPath("");
  std::map<std::string, File *>::iterator it;
  for(it = filelist->begin(); it != filelist->end(); it++) {
    File * file = it->second;
    if (it->first.find(" ") != std::string::npos) {
      continue;
    }
    if (file->isDirectory()) {
      if (getFileListForPath(file->getName()) == NULL) {
        addSubDirectory(file->getName());
      }
    }
  }
}

int SiteRace::getNumUploadedFiles() {
  std::map<std::string, FileList *>::iterator it;
  int sum = 0;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    sum += it->second->getNumUploadedFiles();
  }
  return sum;
}

bool SiteRace::sizeEstimated() {
  return sizeestimated;
}

unsigned long long int SiteRace::getMaxFileSize() {
  std::map<std::string, FileList *>::iterator it;
  unsigned long long int maxsize = 0;
  unsigned long long int maxsizewithfiles = 0;
  for (it = filelists.begin(); it != filelists.end(); it++) {
    unsigned long long int max = it->second->getMaxFileSize();
    if (max > maxsize) {
      maxsize = max;
      if (it->second->getSize() >= 5) {
        maxsizewithfiles = max;
      }
    }
  }
  if (maxsizewithfiles > 0) {
    return maxsizewithfiles;
  }
  return maxsize;
}

bool SiteRace::isDone() {
  return done;
}

void SiteRace::complete() {
  done = true;
  race->reportDone(this);
}

Race * SiteRace::getRace() {
  return race;
}

void SiteRace::reportSize(unsigned int size) {
  if (!sizeestimated) {
    race->reportSize(this, size);
  }
  sizeestimated = true;
}
