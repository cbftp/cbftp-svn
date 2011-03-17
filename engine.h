#include <string>
#include <list>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "scoreboard.h"
#include "globalcontext.h"
#include "sitethread.h"
#include "filelist.h"
#include "sitethreadmanager.h"
#include "transfermanager.h"
#include "race.h"

extern GlobalContext * global;

class Engine {
  private:
    std::list<Race *> races;
    ScoreBoard * scoreboard;
    int maxavgspeed;
    pthread_t thread;
    sem_t race_sem;
    sem_t * list_refresh;
    bool race;
    void refreshScoreBoard();
    void issueOptimalTransfers();
    void setSpeedScale();
    int calculateScore(File *, SiteThread *, SiteRace *, SiteThread *, SiteRace *, int);
  public:
    Engine();
    void newRace(std::string, std::string, std::list<std::string>);
    static void * run(void *);
    void runInstance();
};
