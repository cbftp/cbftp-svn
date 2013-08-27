#include "race.h"

Race::Race(std::string release, std::string section) {
  this->name = release;
  size_t splitpos = name.rfind("-");
  if (splitpos != std::string::npos) {
    this->group = name.substr(splitpos + 1);
  }
  else {
    this->group = "";
  }
  this->section = section;
  done = false;
  maxfilelistsize = 0;
  estimatedsubpaths.push_back("");
}

void Race::addSite(SiteLogic * sitelogic) {
  sites.push_back(sitelogic);
}

std::list<SiteLogic *>::iterator Race::begin() {
  return sites.begin();
}

std::list<SiteLogic *>::iterator Race::end() {
  return sites.end();
}

std::string Race::getName() {
  return name;
}

std::string Race::getGroup() {
  return group;
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

unsigned int Race::guessedSize(std::string subpath) {
  std::map<std::string, unsigned int>::iterator it = guessedsize.find(subpath);
  if (it != guessedsize.end()) {
    return it->second;
  }
  return 10;
}

unsigned long long int Race::guessedFileSize(std::string subpath, std::string file) {
  if (sizelocationtrackers.find(subpath) == sizelocationtrackers.end()) {
    return 0;
  }
  if (sizelocationtrackers[subpath].find(file) == sizelocationtrackers[subpath].end()) {
    return 0;
  }
  return sizelocationtrackers[subpath][file].getEstimatedSize();
}

void Race::prepareGuessedFileList(std::string subpath) {
  guessedfilelist.clear();
  if (sizelocationtrackers.find(subpath) == sizelocationtrackers.end()) {
    return;
  }
  std::map<std::string, SizeLocationTrack>::iterator it;
  int highestnumsites = 0;
  for (it = sizelocationtrackers[subpath].begin(); it != sizelocationtrackers[subpath].end(); it++) {
    int thisnumsites = it->second.numSites();
    if (thisnumsites > highestnumsites) {
      highestnumsites = thisnumsites;
    }
  }
  int minnumsites = highestnumsites / 2;
  for (it = sizelocationtrackers[subpath].begin(); it != sizelocationtrackers[subpath].end(); it++) {
    if (it->second.numSites() > minnumsites) {
      guessedfilelist.push_back(it->first);
    }
  }
}

std::list<std::string>::iterator Race::guessedFileListBegin() {
  return guessedfilelist.begin();
}

std::list<std::string>::iterator Race::guessedFileListEnd() {
  return guessedfilelist.end();
}

bool Race::SFVReported(std::string subpath) {
  std::map<std::string, std::list<SiteRace *> >::iterator it = sfvreports.find(subpath);
  return it != sfvreports.end();
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
    guessedsize[subdir] = 2;
  }
}

void Race::reportSFV(SiteRace * sr, std::string subpath) {
  std::map<std::string, std::list<SiteRace *> >::iterator it = sfvreports.find(subpath);
  if (it == sfvreports.end()) {
    sfvreports[subpath] = std::list<SiteRace *>();
  }
  for (std::list<SiteRace *>::iterator it2 = sfvreports[subpath].begin(); it2 != sfvreports[subpath].end(); it2++) {
    if (*it2 == sr) {
      return;
    }
  }
  sfvreports[subpath].push_back(sr);
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

void Race::reportSize(SiteRace * sr, FileList * fl, std::string subpath, std::list<std::string > * uniques, bool final) {
  std::list<std::string>::iterator itu;
  File * file;
  if (sizelocationtrackers.find(subpath) == sizelocationtrackers.end()) {
    sizelocationtrackers[subpath] = std::map<std::string, SizeLocationTrack>();
  }
  for (itu = uniques->begin(); itu != uniques->end(); itu++) {
    if (sizelocationtrackers[subpath].find(*itu) == sizelocationtrackers[subpath].end()) {
      sizelocationtrackers[subpath][*itu] = SizeLocationTrack();
    }
    if ((file = fl->getFile(*itu)) != NULL) {
      sizelocationtrackers[subpath][*itu].add(sr, file->getSize());
    }
    else {
      sizelocationtrackers[subpath][*itu].add(sr, 0);
    }
  }
  if (sizes.find(sr) == sizes.end()) {
    sizes[sr] = std::map<std::string, unsigned int>();
  }
  sizes[sr][subpath] = uniques->size();
  std::map<SiteRace *, std::map<std::string, unsigned int> >::iterator it;
  std::map<std::string, unsigned int>::iterator it2;
  std::vector<unsigned int> subpathsizes;
  for (it = sizes.begin(); it != sizes.end(); it++) {
    it2 = it->second.find(subpath);
    if (it2 != it->second.end()) {
      subpathsizes.push_back(it2->second);
    }
  }
  std::map<std::string, SizeLocationTrack>::iterator it3;
  int highestnumsites = 0;
  int thisguessedsize = 0;
  for (it3 = sizelocationtrackers[subpath].begin(); it3 != sizelocationtrackers[subpath].end(); it3++) {
    int thisnumsites = it3->second.numSites();
    if (thisnumsites > highestnumsites) {
      highestnumsites = thisnumsites;
    }
  }
  int minnumsites = highestnumsites / 2;
  for (it3 = sizelocationtrackers[subpath].begin(); it3 != sizelocationtrackers[subpath].end(); it3++) {
    if (it3->second.numSites() > minnumsites) {
      thisguessedsize++;
    }
  }
  guessedsize[subpath] = thisguessedsize;
  if (final) {
    // stupid formula, replace with time check from race start
    if (subpathsizes.size() == sites.size() ||
      (subpathsizes.size() >= sites.size() * 0.8 && sites.size() > 2)) {
      std::sort(subpathsizes.begin(), subpathsizes.end());
      estimatedsize[subpath] = thisguessedsize;
    }
  }
}
