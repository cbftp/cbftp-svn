#pragma once

#include <list>
#include <string>

#include "skiplistitem.h"

class SkipList {
private:
  std::list<SkiplistItem> entries;
  bool defaultallow;
  int wildcmp(const char *, const char *) const;
  int wildcmpCase(const char *, const char *) const;
  void addDefaultEntries();
public:
  SkipList();
  void addEntry(std::string, bool, bool, int, bool);
  void clearEntries();
  std::list<SkiplistItem>::const_iterator entriesBegin() const;
  std::list<SkiplistItem>::const_iterator entriesEnd() const;
  bool isAllowed(std::string, bool) const;
  bool isAllowed(std::string, bool, bool) const;
  void readConfiguration();
  void writeState();
  bool defaultAllow() const;
  unsigned int size() const;
  void setDefaultAllow(bool);
};
