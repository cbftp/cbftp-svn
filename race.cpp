#include "race.h"

Race::Race(std::string release, std::string section) {
  this->name = release;
  this->section = section;
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
