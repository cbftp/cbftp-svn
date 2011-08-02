#pragma once
#include <string>
#include <vector>
#include <list>
#include <pthread.h>
#include <semaphore.h>
#include "sitemanager.h"
#include "ftpthread.h"
#include "siterace.h"
#include "globalcontext.h"
#include "ftpthreadcom.h"
#include "scoreboardelement.h"
#include "potentialtracker.h"

#define RAWBUFMAXLEN 1024

extern GlobalContext * global;

class SiteThread {
  private:
    PotentialTracker * ptrack;
    std::vector<RawBuffer *> rawbufs;
    std::vector<FTPThread *> conns;
    std::vector<SiteRace *> races;
    FTPThreadCom * ftpthreadcom;
    pthread_t thread;
    pthread_mutex_t slots;
    sem_t notifysem;
    sem_t * list_refresh;
    int slots_dn;
    int slots_up;
    int available;
    int loggedin;
    Site * site;
    void activate();
    static void * run(void *);
  public:
    SiteThread(std::string);
    void runInstance();
    void addRace(Race *, std::string, std::string);
    Site * getSite();
    SiteRace * getRace(std::string);
    bool getDownloadThread(SiteRace *, std::string, FTPThread **);
    bool getUploadThread(SiteRace *, std::string, FTPThread **);
    bool getReadyThread(SiteRace *, FTPThread **);
    bool getReadyThread(SiteRace *, std::string, FTPThread **, bool, bool);
    bool downloadSlotAvailable();
    bool uploadSlotAvailable();
    int getCurrDown();
    int getCurrUp();
    int getCurrLogins();
    void transferComplete(bool isdownload);
    bool getSlot(bool);
    void pushPotential(int, std::string, SiteThread *);
    bool potentialCheck(int);
};
