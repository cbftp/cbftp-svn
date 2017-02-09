#pragma once

#include <list>
#include <string>
#include <map>

#include "skiplistitem.h"

class Site;

class SkipList {
private:
  std::list<SkiplistItem> entries;
  mutable std::map<std::string, bool> matchcache;
  bool defaultallow;
  const SkipList * globalskip;
  void addDefaultEntries();
public:
  SkipList();
  SkipList(const SkipList *);
  void addEntry(std::string, bool, bool, int, bool);
  void clearEntries();
  std::list<SkiplistItem>::const_iterator entriesBegin() const;
  std::list<SkiplistItem>::const_iterator entriesEnd() const;
  bool isAllowed(const std::string &, const bool) const;
  bool isAllowed(const std::string &, const bool, const bool) const;
  bool isAllowed(const std::string &, const bool, const bool, const SkipList *) const;
  bool defaultAllow() const;
  unsigned int size() const;
  void setDefaultAllow(bool);
  void wipeCache();
  void setGlobalSkip(SkipList *);
};
