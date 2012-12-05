#pragma once

#include <string>
#include <list>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

#include "scoreboard.h"
#include "scoreboardelement.h"
#include "globalcontext.h"
#include "sitethread.h"
#include "filelist.h"
#include "sitethreadmanager.h"
#include "transfermanager.h"
#include "race.h"

#define SPREAD 1

extern GlobalContext * global;

class Engine {
  private:
    std::list<Race *> allraces;
    std::list<Race *> currentraces;
    ScoreBoard * scoreboard;
    int maxavgspeed;
    pthread_t thread;
    sem_t race_sem;
    sem_t * list_refresh;
    void estimateRaceSizes();
    void refreshScoreBoard();
    void issueOptimalTransfers();
    void setSpeedScale();
    int calculateScore(File *, Race *, SiteThread *, SiteRace *, SiteThread *, SiteRace *, int, bool);
  public:
    Engine();
    void newRace(std::string, std::string, std::list<std::string>);
    static void * run(void *);
    void runInstance();
    int currentRaces();
    int allRaces();
    Race * getRace(std::string);
    std::list<Race *>::iterator getRacesIteratorBegin();
    std::list<Race *>::iterator getRacesIteratorEnd();
};
