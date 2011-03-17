#include "siterace.h"

SiteRace::SiteRace(std::string section, std::string release, std::string username) {
  this->section = section;
  this->release = release;
  path = section.append("/").append(release);
  filelist = new FileList(username);
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
