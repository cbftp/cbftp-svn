#include "localfilelist.h"

#include <utility>

LocalFileList::LocalFileList(std::string path) :
  path(path), sizefiles(0) {
}

void LocalFileList::addFile(std::string name, unsigned long long int size, bool isdir, std::string user, std::string group, int year, int month, int day, int hour, int minute) {
  LocalFile file(name, size, isdir, user, group, year, month, day, hour, minute);
  files.insert(std::pair<std::string, LocalFile>(name, file));
  if (!isdir) {
    sizefiles++;
  }
}

void LocalFileList::touchFile(const std::string & name) {
  addFile(name, 512, false, "", "", 0, 0, 0, 0, 0);
}

std::map<std::string, LocalFile>::const_iterator LocalFileList::begin() const {
  return files.begin();
}

std::map<std::string, LocalFile>::const_iterator LocalFileList::end() const {
  return files.end();
}

std::map<std::string, LocalFile>::const_iterator LocalFileList::find(std::string file) const {
  return files.find(file);
}

std::string LocalFileList::getPath() const {
  return path;
}

unsigned int LocalFileList::size() const {
  return files.size();
}

unsigned int LocalFileList::sizeFiles() const {
  return sizefiles;
}
