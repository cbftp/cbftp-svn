#include "siterace.h"

SiteRace::SiteRace(Race * race, std::string section, std::string release, std::string username) {
  this->race = race;
  this->section = section;
  this->release = release;
  path = section.append("/").append(release);
  filelist = new FileList(username, path);
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

FileList * SiteRace::getFileList() {
  return filelist;
}

void SiteRace::updateNumFilesUploaded() {
  race->updateSiteProgress(filelist->getSizeUploaded());
}
