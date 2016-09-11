#pragma once

#include <string>
#include <map>

#include "localfile.h"

class LocalFileList {
public:
  LocalFileList(const std::string &);
  void addFile(const std::string &, unsigned long long int, bool, const std::string &, const std::string &, int, int, int, int, int);
  void touchFile(const std::string &);
  std::map<std::string, LocalFile>::const_iterator begin() const;
  std::map<std::string, LocalFile>::const_iterator end() const;
  std::map<std::string, LocalFile>::const_iterator find(const std::string &) const;
  std::string getPath() const;
  unsigned int size() const;
  unsigned int sizeFiles() const;
private:
  std::map<std::string, LocalFile> files;
  std::map<std::string, std::string> lowercasefilemap;
  std::string path;
  int sizefiles;
};
