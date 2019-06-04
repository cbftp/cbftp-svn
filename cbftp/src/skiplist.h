#pragma once

#include <list>
#include <string>
#include <unordered_map>

#include "skiplistitem.h"

class Site;

struct SkipListMatch {
  SkipListMatch(SkipListAction action, bool matched, const std::string & matchpattern);
  SkipListAction action;
  bool matched;
  std::string matchpattern;
};

class SkipList {
private:
  std::list<SkiplistItem> entries;
  mutable std::unordered_map<std::string, SkipListMatch> matchcache;
  const SkipList * globalskip;
  void addDefaultEntries();
  SkipListMatch fallThrough(const std::string & element, const bool dir, const bool inrace, const SkipList * fallthrough) const;
public:
  SkipList();
  SkipList(const SkipList *);
  void addEntry(bool regex, const std::string & pattern, bool file, bool dir, int scope, SkipListAction action);
  void clearEntries();
  std::list<SkiplistItem>::const_iterator entriesBegin() const;
  std::list<SkiplistItem>::const_iterator entriesEnd() const;
  SkipListMatch check(const std::string & element, const bool dir, const bool inrace = true, const SkipList * fallthrough = nullptr) const;
  unsigned int size() const;
  void wipeCache();
  void setGlobalSkip(SkipList *);
};
