#pragma once

#include <string>

class SkiplistItem {
public:
  SkiplistItem(std::string, bool, bool, bool);
  const std::string & matchPattern() const;
  bool matchFile() const;
  bool matchDir() const;
  bool isAllowed() const;
private:
  std::string pattern;
  bool file;
  bool dir;
  bool allow;
};
