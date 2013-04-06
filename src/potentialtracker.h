#pragma once

#include <string>
#include <list>

#include "globalcontext.h"
#include "potentialelement.h"
#include "potentiallistelement.h"
#include "tickpoke.h"
#include "eventreceiver.h"

#define POTENTIALITY_LIFESPAN 3000
#define POTENTIALITY_SLICES 10

extern GlobalContext * global;

class SiteThread;

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
    std::list<PotentialElement *>::iterator findFirstOfSite(SiteThread *);
    bool allTopSlotsUsedForSite(PotentialElement *);
};
