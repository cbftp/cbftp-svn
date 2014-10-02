#pragma once

#include <list>
#include <string>

class SkipList {
private:
  std::list<std::string> entries;
  int wildcmp(const char *, const char *) const;
  int wildcmpCase(const char *, const char *) const;
  void addDefaultEntries();
public:
  SkipList();
  void addEntry(std::string);
  void clearEntries();
  std::list<std::string>::const_iterator entriesBegin() const;
  std::list<std::string>::const_iterator entriesEnd() const;
  bool isAllowed(std::string) const;
  void readConfiguration();
  void writeState();
};
