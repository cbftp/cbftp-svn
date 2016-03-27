#pragma once

#include <string>
#include <list>
#include <vector>

#include "core/eventreceiver.h"

#define POTENTIALITY_LIFESPAN 3000
#define POTENTIALITY_SLICES 10

class GlobalContext;
class PotentialElement;
class PotentialListElement;

extern GlobalContext * global;

class SiteLogic;

class PotentialTracker : private EventReceiver {
  private:
    std::list<PotentialListElement *> potentiallist;
    std::list<PotentialElement *> top;
    void tick(int);
  public:
    PotentialTracker(int);
    ~PotentialTracker();
    int getMaxAvailablePotential();
    PotentialListElement * getFront() const;
    std::list<PotentialElement *>::iterator findFirstOfSite(SiteLogic *);
    bool allTopSlotsUsedForSite(PotentialElement *) const;
};
