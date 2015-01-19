#include "localfile.h"

LocalFile::LocalFile(std::string name, unsigned long long int size, bool isdir) :
  name(name),
  size(size),
  isdir(isdir) {
}

std::string LocalFile::getName() const {
  return name;
}

unsigned long long int LocalFile::getSize() const {
  return size;
}

bool LocalFile::isDirectory() const {
  return isdir;
}
