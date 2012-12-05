#include "race.h"

Race::Race(std::string release, std::string section) {
  this->name = release;
  this->section = section;
  sizeestimated = false;
  done = false;
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

bool Race::sizeEstimated() {
  return sizeestimated;
}

int Race::estimatedSize() {
  return estimatedsize;
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

void Race::reportSize(SiteRace * sr, unsigned int size) {
  if (sizes.find(sr) == sizes.end()) {
    sizes[sr] = size;
  }
  // stupid formula, replace with time check from race start
  if (sizes.size() == sites.size() ||
    (sizes.size() >= sites.size() * 0.8 && sites.size() > 2)) {
    std::vector<unsigned int> singledsizes;
    std::map<SiteRace *, unsigned int>::iterator it;
    for (it = sizes.begin(); it != sizes.end(); it++) {
      singledsizes.push_back(it->second);
    }
    std::sort(singledsizes.begin(), singledsizes.end());
    estimatedsize = singledsizes[singledsizes.size()/2];
    sizeestimated = true;
  }

}
