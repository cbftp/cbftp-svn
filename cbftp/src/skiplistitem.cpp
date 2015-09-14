#include "skiplistitem.h"

SkiplistItem::SkiplistItem(std::string pattern, bool file, bool dir, int scope, bool allow) :
  pattern(pattern), file(file), dir(dir), scope(scope), allow(allow) {
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

bool SkiplistItem::isAllowed() const {
  return allow;
}

int SkiplistItem::matchScope() const {
  return scope;
}
