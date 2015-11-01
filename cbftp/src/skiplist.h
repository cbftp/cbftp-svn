#pragma once

#include <list>
#include <string>
#include <map>

#include "skiplistitem.h"

class SkipList {
private:
  std::list<SkiplistItem> entries;
  mutable std::map<std::string, bool> matchcache;
  bool defaultallow;
  int wildcmp(const char *, const char *) const;
  int wildcmpCase(const char *, const char *) const;
  bool fixedSlashCompare(const std::string &, const std::string &, bool) const;
  std::string createCacheToken(const std::string &, const bool, const bool) const;
  void addDefaultEntries();
public:
  SkipList();
  void addEntry(std::string, bool, bool, int, bool);
  void clearEntries();
  std::list<SkiplistItem>::const_iterator entriesBegin() const;
  std::list<SkiplistItem>::const_iterator entriesEnd() const;
  bool isAllowed(const std::string &, const bool) const;
  bool isAllowed(std::string, const bool, const bool) const;
  bool defaultAllow() const;
  unsigned int size() const;
  void setDefaultAllow(bool);
};
