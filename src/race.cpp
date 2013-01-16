#include "race.h"

Race::Race(std::string release, std::string section) {
  this->name = release;
  this->section = section;
  done = false;
  estimatedsubpaths.push_back("");
}

void Race::addSite(SiteThread * sitethread) {
  sites.push_back(sitethread);
}

std::list<SiteThread *>::iterator Race::begin() {
  return sites.begin();
}

std::list<SiteThread *>::iterator Race::end() {
  return sites.end();
}

std::string Race::getName() {
  return name;
}

std::string Race::getSection() {
  return section;
}

int Race::numSites() {
  return sites.size();
}

bool Race::sizeEstimated(std::string subpath) {
  return estimatedsize.find(subpath) != estimatedsize.end();
}

unsigned int Race::estimatedSize(std::string subpath) {
  std::map<std::string, unsigned int>::iterator it = estimatedsize.find(subpath);
  if (it != estimatedsize.end()) {
    return it->second;
  }
  return 0;
}

std::list<std::string> Race::getSubPaths() {
  return estimatedsubpaths;
}

void Race::updateSiteProgress(int in) {
  if (maxfilelistsize < in) maxfilelistsize = in;
}

int Race::getMaxSiteProgress() {
  return maxfilelistsize;
}

bool Race::isDone() {
  return done;
}

void Race::reportNewSubDir(SiteRace * sr, std::string subdir) {
  if (subpathoccurences.find(subdir) == subpathoccurences.end()) {
    subpathoccurences[subdir] = std::list<SiteRace *>();
  }
  std::list<SiteRace *>::iterator it;
  for (it = subpathoccurences[subdir].begin(); it != subpathoccurences[subdir].end(); it++) {
    if (*it == sr) {
      return;
    }
  }
  subpathoccurences[subdir].push_back(sr);
  if (subpathoccurences[subdir].size() >= sites.size() * 0.5) {
    std::list<std::string>::iterator it2;
    for (it2 = estimatedsubpaths.begin(); it2 != estimatedsubpaths.end(); it2++) {
      if (*it2 == subdir) {
        return;
      }
    }
    estimatedsubpaths.push_back(subdir);
  }
}

void Race::reportDone(SiteRace * sr) {
  std::list<SiteRace *>::iterator it;
  for (it = donesites.begin(); it != donesites.end(); it++) {
    if (*it == sr) {
      return;
    }
  }
  donesites.push_back(sr);
  if (donesites.size() == sites.size()) {
    done = true;
  }
}

void Race::reportSize(SiteRace * sr, std::string subpath, unsigned int size) {
  if (sizes.find(sr) == sizes.end()) {
    sizes[sr] = std::map<std::string, unsigned int>();
  }
  sizes[sr][subpath] = size;
  std::map<SiteRace *, std::map<std::string, unsigned int> >::iterator it;
  std::map<std::string, unsigned int>::iterator it2;
  std::vector<unsigned int> subpathsizes;
  for (it = sizes.begin(); it != sizes.end(); it++) {
    it2 = it->second.find(subpath);
    if (it2 != it->second.end()) {
      subpathsizes.push_back(it2->second);
    }
  }
  // stupid formula, replace with time check from race start
  if (subpathsizes.size() == sites.size() ||
    (subpathsizes.size() >= sites.size() * 0.8 && sites.size() > 2)) {
    std::sort(subpathsizes.begin(), subpathsizes.end());
    estimatedsize[subpath] = subpathsizes[subpathsizes.size()/2];
  }
}
