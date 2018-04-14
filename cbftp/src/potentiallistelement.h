#pragma once

#include <string>
#include <vector>

#include "core/pointer.h"

class SiteLogic;
class PotentialElement;

class PotentialListElement {
  private:
    std::vector<PotentialElement *> slots;
    bool allSlotsUsedForSite(const Pointer<SiteLogic> &, int) const;
  public:
    PotentialListElement(int);
    ~PotentialListElement();
    bool update(const Pointer<SiteLogic> &, int, int, const std::string &);
    void reset();
    std::vector<PotentialElement *> & getSlotsVector();
    void updateSlots(int);

};
