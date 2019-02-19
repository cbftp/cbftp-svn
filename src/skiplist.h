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
  bool defaultallow;
  const SkipList * globalskip;
  mutable std::list<std::string> similarpatterns;
  mutable bool similarpatternsset;
  void addDefaultEntries();
public:
  SkipList();
  SkipList(const SkipList *);
  void addEntry(std::string, bool, bool, int, SkipListAction);
  void clearEntries();
  std::list<SkiplistItem>::const_iterator entriesBegin() const;
  std::list<SkiplistItem>::const_iterator entriesEnd() const;
  SkipListMatch check(const std::string & element, const bool dir, const bool inrace = true, const SkipList * fallthrough = nullptr) const;
  bool defaultAllow() const;
  unsigned int size() const;
  void setDefaultAllow(bool);
  void wipeCache();
  void setGlobalSkip(SkipList *);
  const std::list<std::string> & getSimilarPatterns(const SkipList * fallthrough = nullptr) const;
};
