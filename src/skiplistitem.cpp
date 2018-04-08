#include "skiplistitem.h"

SkiplistItem::SkiplistItem(std::string pattern, bool file, bool dir, int scope, SkipListAction action) :
  pattern(pattern), file(file), dir(dir), scope(scope), action(action) {
}

const std::string & SkiplistItem::matchPattern() const {
  return pattern;
}

bool SkiplistItem::matchFile() const {
  return file;
}

bool SkiplistItem::matchDir() const {
  return dir;
}

SkipListAction SkiplistItem::getAction() const {
  return action;
}

int SkiplistItem::matchScope() const {
  return scope;
}
