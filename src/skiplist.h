#pragma once

#include <stdlib.h>
#include <list>
#include <vector>

#include "globalcontext.h"
#include "datafilehandler.h"

extern GlobalContext * global;

class SkipList {
private:
  std::list<std::string> entries;
  int wildcmp(const char *, const char *);
public:
  SkipList();
  void addEntry(std::string);
  void clearEntries();
  std::list<std::string>::iterator entriesBegin();
  std::list<std::string>::iterator entriesEnd();
  bool isAllowed(std::string);
  void readConfiguration();
  void writeState();
};
