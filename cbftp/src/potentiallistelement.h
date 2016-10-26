#pragma once

#include <string>
#include <vector>

#include "core/pointer.h"

class SiteLogic;
class PotentialElement;

class PotentialListElement {
  private:
    std::vector<PotentialElement *> slots;
  public:
    PotentialListElement(int);
    ~PotentialListElement();
    void update(const Pointer<SiteLogic> &, int, int, const std::string &);
    void reset();
    std::vector<PotentialElement *> & getSlotsVector();
    void updateSlots(int);
    bool allSlotsUsedForSite(const Pointer<SiteLogic> &, int) const;
};
