#pragma once

#include <string>

#define SCOPE_IN_RACE 0
#define SCOPE_ALL 1

class SkiplistItem {
public:
  SkiplistItem(std::string, bool, bool, int, bool);
  const std::string & matchPattern() const;
  bool matchFile() const;
  bool matchDir() const;
  bool isAllowed() const;
  int matchScope() const;
private:
  std::string pattern;
  bool file;
  bool dir;
  int scope;
  bool allow;
};
