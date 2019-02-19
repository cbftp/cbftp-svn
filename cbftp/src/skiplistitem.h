#pragma once

#include <string>

enum Scope {
  SCOPE_IN_RACE = 0,
  SCOPE_ALL = 1
};

enum SkipListAction {
  SKIPLIST_ALLOW,
  SKIPLIST_DENY,
  SKIPLIST_UNIQUE,
  SKIPLIST_SIMILAR
};

class SkiplistItem {
public:
  SkiplistItem(std::string, bool, bool, int, SkipListAction);
  const std::string & matchPattern() const;
  bool matchFile() const;
  bool matchDir() const;
  SkipListAction getAction() const;
  int matchScope() const;
private:
  std::string pattern;
  bool file;
  bool dir;
  int scope;
  SkipListAction action;
};
