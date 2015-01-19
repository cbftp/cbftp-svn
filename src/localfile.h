#pragma once

#include <string>

class LocalFile {
public:
  LocalFile(std::string, unsigned long long int, bool);
  std::string getName() const;
  unsigned long long int getSize() const;
  bool isDirectory() const;
private:
  std::string name;
  unsigned long long int size;
  bool isdir;
};
