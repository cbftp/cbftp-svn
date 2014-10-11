#include "skiplistitem.h"

SkiplistItem::SkiplistItem(std::string pattern, bool file, bool dir, bool allow) :
  pattern(pattern), file(file), dir(dir), allow(allow) {
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
