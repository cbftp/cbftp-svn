#include "siterace.h"

SiteRace::SiteRace(Race * race, std::string section, std::string release, std::string username) {
  this->race = race;
  this->section = section;
  this->release = release;
  sizeestimated = false;
  path = section.append("/").append(release);
  filelist = new FileList(username, path);
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

FileList * SiteRace::getFileList() {
  return filelist;
}

void SiteRace::updateNumFilesUploaded() {
  race->updateSiteProgress(filelist->getSizeUploaded());
}

bool SiteRace::sizeEstimated() {
  return sizeestimated;
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
