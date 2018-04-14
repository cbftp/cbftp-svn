#pragma once

#include <string>
#include <list>
#include <vector>

#include "core/pointer.h"
#include "core/eventreceiver.h"

#define POTENTIALITY_LIFESPAN 3000
#define POTENTIALITY_SLICES 10

class PotentialElement;
class PotentialListElement;
class SiteLogic;

class PotentialTracker : private EventReceiver {
  private:
    std::list<PotentialListElement *> potentiallist;
    std::list<PotentialElement *> top;
    bool maxpotentialcalculated;
    void tick(int);
    bool allTopSlotsUsedForSite(PotentialElement *) const;
    std::list<PotentialElement *>::iterator findFirstOfSite(const Pointer<SiteLogic> &);
    void calculateMaxAvailablePotential();
  public:
    PotentialTracker(int);
    ~PotentialTracker();
    int getMaxAvailablePotential();
    void pushPotential(int, const std::string &, const Pointer<SiteLogic> &, int);
    void updateSlots(int);
};
