#pragma once

#include <string>
#include <map>

#include "localfile.h"

class LocalFileList {
public:
  LocalFileList(std::string);
  void addFile(std::string, unsigned long long int, bool);
  std::map<std::string, LocalFile>::const_iterator begin() const;
  std::map<std::string, LocalFile>::const_iterator end() const;
  std::map<std::string, LocalFile>::const_iterator find(std::string) const;
  std::string getPath() const;
private:
  std::map<std::string, LocalFile> files;
  std::string path;
};
