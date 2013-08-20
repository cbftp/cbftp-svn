#include "skiplist.h"

SkipList::SkipList() {

}

int SkipList::wildcmp(const char *wild, const char *string) {
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

int SkipList::wildcmpCase(const char *wild, const char *string) {
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

std::list<std::string>::iterator SkipList::entriesBegin() {
  return entries.begin();
}

std::list<std::string>::iterator SkipList::entriesEnd() {
  return entries.end();
}

bool SkipList::isAllowed(std::string element) {
  std::list<std::string>::iterator it;
  for (it = entries.begin(); it != entries.end(); it++) {
    if (wildcmp(it->c_str(), element.c_str())) {
      return false;
    }
  }
  return true;
}

void SkipList::readConfiguration() {
  std::vector<std::string> lines;
  entries.clear();
  global->getDataFileHandler()->getDataFor("SkipList", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("entry")) {
      entries.push_back(value);
    }
  }
}

void SkipList::writeState() {
  DataFileHandler * filehandler = global->getDataFileHandler();
  std::list<std::string>::iterator it;
  for (it = entries.begin(); it != entries.end(); it++) {
    filehandler->addOutputLine("SkipList", "entry=" + *it);
  }
}
