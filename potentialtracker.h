#pragma once
#include <string>
#include <list>
#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include "globalcontext.h"
#include "potentialelement.h"
#include "potentiallistelement.h"

#define POTENTIALITY_LIFESPAN 3000
#define POTENTIALITY_SLICES 10

extern GlobalContext * global;

class SiteThread;

class PotentialTracker {
  private:
    std::list<PotentialListElement *> potentiallist;
    std::list<PotentialElement *> top;
    pthread_t thread;
    pthread_t tickthread;
    pthread_mutex_t listmutex;
    sem_t tick;
    std::list<PotentialListElement *>::iterator itple;
    std::vector<PotentialElement *>::iterator itpe;
    std::list<PotentialElement *>::iterator ittop;
    std::list<PotentialElement *>::iterator ittop2;
  public:
    PotentialTracker(int);
    void runInstance();
    void runTickInstance();
    int getMaxAvailablePotential();
    PotentialListElement * getFront();
    std::list<PotentialElement *>::iterator findFirstOfSite(SiteThread *);
    bool allTopSlotsUsedForSite(PotentialElement *);
    void postTick();
};

void * runPotentialTracker(void *);
void * runTickPotentialTracker(void *);
