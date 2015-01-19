#include "localfilelist.h"

#include <utility>

LocalFileList::LocalFileList(std::string path) :
  path(path) {
}

void LocalFileList::addFile(std::string name, unsigned long long int size, bool isdir) {
  LocalFile file(name, size, isdir);
  files.insert(std::pair<std::string, LocalFile>(name, file));
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
