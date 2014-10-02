#include "skiplist.h"

#include <stdlib.h>
#include <vector>

#include "globalcontext.h"
#include "datafilehandler.h"

extern GlobalContext * global;

SkipList::SkipList() {

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

void SkipList::addEntry(std::string entry) {
  entries.push_back(entry);
}

void SkipList::clearEntries() {
  entries.clear();
}

std::list<std::string>::const_iterator SkipList::entriesBegin() const {
  return entries.begin();
}

std::list<std::string>::const_iterator SkipList::entriesEnd() const {
  return entries.end();
}

bool SkipList::isAllowed(std::string element) const {
  std::list<std::string>::const_iterator it;
  for (it = entries.begin(); it != entries.end(); it++) {
    if (wildcmp(it->c_str(), element.c_str())) {
      return false;
    }
  }
  return true;
}

void SkipList::addDefaultEntries() {
  entries.push_back("* *");
  entries.push_back("*%*");
  entries.push_back("*[*");
  entries.push_back("*]*");
  entries.push_back("*-missing");
  entries.push_back("*-offline");
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
      entries.push_back(value);
    }
    if (!setting.compare("initialized")) {
      initialized = true;
    }
  }
  if (!initialized && !entries.size()) {
    addDefaultEntries();
  }
}

void SkipList::writeState() {
  DataFileHandler * filehandler = global->getDataFileHandler();
  std::list<std::string>::iterator it;
  for (it = entries.begin(); it != entries.end(); it++) {
    filehandler->addOutputLine("SkipList", "entry=" + *it);
  }
  filehandler->addOutputLine("SkipList", "initialized=true");
}
