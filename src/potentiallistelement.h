#pragma once

#include <string>
#include <vector>

class SiteLogic;
class PotentialElement;

class PotentialListElement {
  private:
    std::vector<PotentialElement *> slots;
  public:
    PotentialListElement(int);
    void update(SiteLogic *, int, int, int, std::string);
    void reset();
    std::vector<PotentialElement *> & getSlotsVector();
    bool allThreadsUsedForSite(SiteLogic *, int) const;
};
