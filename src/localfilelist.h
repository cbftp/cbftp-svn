#pragma once

#include <string>
#include <map>

#include "localfile.h"
#include "path.h"

class LocalFileList {
public:
  LocalFileList(const Path &);
  void addFile(const std::string &, unsigned long long int, bool, const std::string &, const std::string &, int, int, int, int, int);
  void addFile(const LocalFile & file);
  void touchFile(const std::string &);
  std::map<std::string, LocalFile>::const_iterator begin() const;
  std::map<std::string, LocalFile>::const_iterator end() const;
  std::map<std::string, LocalFile>::const_iterator find(const std::string &) const;
  const Path & getPath() const;
  unsigned int size() const;
  unsigned int sizeFiles() const;
private:
  std::map<std::string, LocalFile> files;
  std::map<std::string, std::string> lowercasefilemap;
  Path path;
  int sizefiles;
};
