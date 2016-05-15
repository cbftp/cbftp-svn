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
    ~PotentialListElement();
    void update(SiteLogic *, int, int, const std::string &);
    void reset();
    std::vector<PotentialElement *> & getSlotsVector();
    void updateSlots(int);
    bool allSlotsUsedForSite(SiteLogic *, int) const;
};
