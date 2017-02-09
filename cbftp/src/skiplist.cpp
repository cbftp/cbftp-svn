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

SkipList::SkipList() : defaultallow(true), globalskip(NULL) {
  addDefaultEntries();
}

SkipList::SkipList(const SkipList * globalskip) : defaultallow(true), globalskip(globalskip) {
  util::assert(globalskip != NULL);
}

void SkipList::addEntry(std::string pattern, bool file, bool dir, int scope, bool allow) {
  entries.push_back(SkiplistItem(pattern, file, dir, scope, allow));
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

bool SkipList::isAllowed(const std::string & element, const bool dir) const {
  return isAllowed(element, dir, true);
}

bool SkipList::isAllowed(const std::string & element, const bool dir, const bool inrace) const {
  return isAllowed(element, dir, inrace, NULL);
}

bool SkipList::isAllowed(const std::string & element, const bool dir, const bool inrace, const SkipList * fallthrough) const {
  std::string cachetoken = createCacheToken(element, dir, inrace);
  std::map<std::string, bool>::const_iterator mit = matchcache.find(cachetoken);
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
          bool allowed = it->isAllowed();
          matchcache[cachetoken] = allowed;
          return allowed;
        }
      }
    }
  }
  bool res = fallthrough ?
             fallthrough->isAllowed(element, dir, inrace) :
             (globalskip ? globalskip->isAllowed(element, dir, inrace) : defaultallow);
  matchcache[cachetoken] = res;
  return res;
}

void SkipList::addDefaultEntries() {
  entries.push_back(SkiplistItem("* *", true, true, SCOPE_ALL, false));
  entries.push_back(SkiplistItem("*%*", true, true, SCOPE_IN_RACE, false));
  entries.push_back(SkiplistItem("*[*", true, true, SCOPE_IN_RACE, false));
  entries.push_back(SkiplistItem("*]*", true, true, SCOPE_IN_RACE, false));
  entries.push_back(SkiplistItem("*-missing", true, false, SCOPE_IN_RACE, false));
  entries.push_back(SkiplistItem("*-offline", true, false, SCOPE_IN_RACE, false));
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
