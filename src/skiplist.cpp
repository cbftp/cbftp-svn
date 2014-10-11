#include "skiplist.h"

#include <stdlib.h>
#include <vector>

#include "globalcontext.h"
#include "datafilehandler.h"

extern GlobalContext * global;

SkipList::SkipList() : defaultallow(true) {

}

int SkipList::wildcmp(const char *wild, const char *string) const {
  const char *cp = NULL, *mp = NULL;
  while ((*string) && (*wild != '*')) {
    if (*wild != *string && *wild != '?' &&
        !(*wild >= 65 && *wild <= 90 && *wild + 32 == *string) &&
        !(*wild >= 97 && *wild <= 122 && *wild - 32 == *string)) {
      return 0;
    }
    wild++;
    string++;
  }
  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if (*wild == *string || *wild == '?' ||
    (*wild >= 65 && *wild <= 90 && *wild + 32 == *string) ||
    (*wild >= 97 && *wild <= 122 && *wild - 32 == *string)) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }
  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

int SkipList::wildcmpCase(const char *wild, const char *string) const {
  const char *cp = NULL, *mp = NULL;
  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return 0;
    }
    wild++;
    string++;
  }
  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return 1;
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }
  while (*wild == '*') {
    wild++;
  }
  return !*wild;
}

void SkipList::addEntry(std::string pattern, bool file, bool dir, int scope, bool allow) {
  entries.push_back(SkiplistItem(pattern, file, dir, scope, allow));
}

void SkipList::clearEntries() {
  entries.clear();
}

std::list<SkiplistItem>::const_iterator SkipList::entriesBegin() const {
  return entries.begin();
}

std::list<SkiplistItem>::const_iterator SkipList::entriesEnd() const {
  return entries.end();
}

bool SkipList::isAllowed(std::string element, bool dir) const {
  return isAllowed(element, dir, true);
}

bool SkipList::isAllowed(std::string element, bool dir, bool inrace) const {
  std::list<SkiplistItem>::const_iterator it;
  for (it = entries.begin(); it != entries.end(); it++) {
    if (it->matchScope() == SCOPE_IN_RACE && !inrace) {
      continue;
    }
    if ((it->matchDir() && dir) || (it->matchFile() && !dir)) {
      if (wildcmp(it->matchPattern().c_str(), element.c_str())) {
        return it->isAllowed();
      }
    }
  }
  return defaultallow;
}

void SkipList::addDefaultEntries() {
  entries.push_back(SkiplistItem("* *", true, true, SCOPE_IN_RACE, false));
  entries.push_back(SkiplistItem("*%*", true, true, SCOPE_IN_RACE, false));
  entries.push_back(SkiplistItem("*[*", true, true, SCOPE_IN_RACE, false));
  entries.push_back(SkiplistItem("*]*", true, true, SCOPE_IN_RACE, false));
  entries.push_back(SkiplistItem("*-missing", true, false, SCOPE_IN_RACE, false));
  entries.push_back(SkiplistItem("*-offline", true, false, SCOPE_IN_RACE, false));
}

void SkipList::readConfiguration() {
  std::vector<std::string> lines;
  entries.clear();
  global->getDataFileHandler()->getDataFor("SkipList", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  bool initialized = false;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("entry")) {
      size_t split = value.find('$');
      if (split != std::string::npos) {
        std::string pattern = value.substr(0, split);
        value = value.substr(split + 1);
        split = value.find('$');
        bool file = value.substr(0, split) == "true" ? true : false;
        value = value.substr(split + 1);
        split = value.find('$');
        bool dir = value.substr(0, split) == "true" ? true : false;
        value = value.substr(split + 1);
        split = value.find('$');
        bool allowed;
        int scope;
        if (split != std::string::npos) {
          scope = global->str2Int(value.substr(0, split));
          allowed = value.substr(split + 1) == "true" ? true : false;
        }
        else {
          scope = SCOPE_IN_RACE;
          allowed = value == "true" ? true : false;
        }
        entries.push_back(SkiplistItem(pattern, file, dir, scope, allowed));
      }
      else { // backwards compatibility
        entries.push_back(SkiplistItem(value, true, true, SCOPE_IN_RACE, false));
      }
    }
    if (!setting.compare("initialized")) {
      initialized = true;
    }
    if (!setting.compare("defaultallow")) {
      defaultallow = value.compare("true") ? true : false;
    }
  }
  if (!initialized && !entries.size()) {
    addDefaultEntries();
  }
}

void SkipList::writeState() {
  DataFileHandler * filehandler = global->getDataFileHandler();
  std::list<SkiplistItem>::const_iterator it;
  for (it = entries.begin(); it != entries.end(); it++) {
    std::string entryline = it->matchPattern() + "$" +
        (it->matchFile() ? "true" : "false") + "$" +
        (it->matchDir() ? "true" : "false") + "$" +
        global->int2Str(it->matchScope()) + "$" +
        (it->isAllowed() ? "true" : "false");
    filehandler->addOutputLine("SkipList", "entry=" + entryline);
  }
  filehandler->addOutputLine("SkipList", "initialized=true");
  std::string defaultallowstr = defaultallow ? "true" : "false";
  filehandler->addOutputLine("SkipList", "defaultallow=" + defaultallowstr);
}

unsigned int SkipList::size() const {
  return entries.size();
}

bool SkipList::defaultAllow() const {
  return defaultallow;
}

void SkipList::setDefaultAllow(bool defaultallow) {
  this->defaultallow = defaultallow;
}
