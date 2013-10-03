#pragma once

#include <string>
#include <list>
#include <pthread.h>
#include <vector>

#include "eventreceiver.h"

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
    pthread_mutex_t listmutex;
    std::list<PotentialListElement *>::iterator itple;
    std::vector<PotentialElement *>::iterator itpe;
    std::list<PotentialElement *>::iterator ittop;
    std::list<PotentialElement *>::iterator ittop2;
    void tick(int);
  public:
    PotentialTracker(int);
    ~PotentialTracker();
    int getMaxAvailablePotential();
    PotentialListElement * getFront();
    std::list<PotentialElement *>::iterator findFirstOfSite(SiteLogic *);
    bool allTopSlotsUsedForSite(PotentialElement *);
};
