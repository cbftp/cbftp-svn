#include "skiplist.h"

#include <cstdlib>
#include <vector>

#include "util.h"
#include "globalcontext.h"
#include "engine.h"

namespace {

bool fixedSlashCompare(const std::string & wildpattern, const std::string & element, bool casesensitive) {
  size_t wildslashpos = wildpattern.find('/');
  size_t elemslashpos = element.find('/');
  if (wildslashpos != std::string::npos && elemslashpos != std::string::npos) {
    std::string wild = wildpattern.substr(0, wildslashpos);
    std::string elem = element.substr(0, elemslashpos);
    if (!casesensitive) {
      if (!util::wildcmp(wild.c_str(), elem.c_str())) {
        return false;
      }
    }
    else {
      if (!util::wildcmpCase(wild.c_str(), elem.c_str())) {
        return false;
      }
    }
    return fixedSlashCompare(wildpattern.substr(wildslashpos + 1), element.substr(elemslashpos + 1), casesensitive);
  }
  else if (wildslashpos == std::string::npos && elemslashpos == std::string::npos) {
    if (!casesensitive) {
      return util::wildcmp(wildpattern.c_str(), element.c_str());
    }
    else {
      return util::wildcmpCase(wildpattern.c_str(), element.c_str());
    }
  }
  else {
    return false;
  }
}

std::string createCacheToken(const std::string & pattern, const bool dir, const bool inrace) {
  return pattern + (dir ? "1" : "0") + (inrace ? "1" : "0");
}

}

SkipListMatch::SkipListMatch(SkipListAction action, bool matched, const std::string & matchpattern) : action(action), matched(matched), matchpattern(matchpattern) {

}

SkipList::SkipList() : defaultallow(true), globalskip(NULL) {
  addDefaultEntries();
}

SkipList::SkipList(const SkipList * globalskip) : defaultallow(true), globalskip(globalskip) {
  util::assert(globalskip != NULL);
}

void SkipList::addEntry(std::string pattern, bool file, bool dir, int scope, SkipListAction action) {
  entries.push_back(SkiplistItem(pattern, file, dir, scope, action));
  wipeCache();
}

void SkipList::clearEntries() {
  entries.clear();
  wipeCache();
}

std::list<SkiplistItem>::const_iterator SkipList::entriesBegin() const {
  return entries.begin();
}

std::list<SkiplistItem>::const_iterator SkipList::entriesEnd() const {
  return entries.end();
}

SkipListMatch SkipList::check(const std::string & element, const bool dir) const {
  return check(element, dir, true);
}

SkipListMatch SkipList::check(const std::string & element, const bool dir, const bool inrace) const {
  return check(element, dir, inrace, NULL);
}

SkipListMatch SkipList::check(const std::string & element, const bool dir, const bool inrace, const SkipList * fallthrough) const {
  std::string cachetoken = createCacheToken(element, dir, inrace);
  std::map<std::string, SkipListMatch>::const_iterator mit = matchcache.find(cachetoken);
  if (mit != matchcache.end()) {
    return mit->second;
  }
  std::list<SkiplistItem>::const_iterator it;
  std::list<std::string> elementparts;
  std::list<std::string>::iterator partsit;
  elementparts.push_back(element);
  size_t slashpos;
  std::string elementmp = element;
  while ((slashpos = elementmp.find('/')) != std::string::npos) {
    elementmp = elementmp.substr(slashpos + 1);
    if (elementmp.length()) {
      elementparts.push_back(elementmp);
    }
  }
  for (partsit = elementparts.begin(); partsit != elementparts.end(); partsit++) {
    for (it = entries.begin(); it != entries.end(); it++) {
      if (it->matchScope() == SCOPE_IN_RACE && !inrace) {
        continue;
      }
      if ((it->matchDir() && dir) || (it->matchFile() && !dir)) {
        if (fixedSlashCompare(it->matchPattern(), *partsit, false)) {
          SkipListMatch match(it->getAction(), true, it->matchPattern());
          matchcache.insert(std::pair<std::string, SkipListMatch>(cachetoken, match));
          return match;
        }
      }
    }
  }
  SkipListMatch match(defaultallow ? SKIPLIST_ALLOW : SKIPLIST_DENY, false, "");
  if (fallthrough) {
    match = fallthrough->check(element, dir, inrace);
  }
  else if (globalskip) {
      match = globalskip->check(element, dir, inrace);
  }
  matchcache.insert(std::pair<std::string, SkipListMatch>(cachetoken, match));
  return match;
}

void SkipList::addDefaultEntries() {
  entries.push_back(SkiplistItem("* *", true, true, SCOPE_ALL, SKIPLIST_DENY));
  entries.push_back(SkiplistItem("*%*", true, true, SCOPE_IN_RACE, SKIPLIST_DENY));
  entries.push_back(SkiplistItem("*[*", true, true, SCOPE_IN_RACE, SKIPLIST_DENY));
  entries.push_back(SkiplistItem("*]*", true, true, SCOPE_IN_RACE, SKIPLIST_DENY));
}

unsigned int SkipList::size() const {
  return entries.size();
}

bool SkipList::defaultAllow() const {
  return defaultallow;
}

void SkipList::setDefaultAllow(bool defaultallow) {
  this->defaultallow = defaultallow;
  wipeCache();
}

void SkipList::wipeCache() {
  matchcache.clear();
  if (!globalskip) {
    global->getEngine()->clearSkipListCaches();
  }
}

void SkipList::setGlobalSkip(SkipList * skiplist) {
  globalskip = skiplist;
}
