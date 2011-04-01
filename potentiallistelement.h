#pragma once
#include <string>
#include <vector>
#include "potentialelement.h"

class SiteThread;

class PotentialListElement {
  private:
    std::vector<PotentialElement *> slots;
  public:
    PotentialListElement(int);
    void update(SiteThread *, int, int, int, std::string);
    void reset();
    std::vector<PotentialElement *> & getSlotsVector();
    bool allThreadsUsedForSite(SiteThread *, int);
};
