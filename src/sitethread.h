#pragma once

#include <string>
#include <vector>
#include <list>
#include <pthread.h>
#include <semaphore.h>

#include "sitemanager.h"
#include "ftpthread.h"
#include "filelist.h"
#include "siterace.h"
#include "globalcontext.h"
#include "ftpthreadcom.h"
#include "scoreboardelement.h"
#include "potentialtracker.h"
#include "sitethreadrequest.h"
#include "sitethreadrequestready.h"
#include "ui/uicommunicator.h"
#include "tickpoke.h"
#include "connstatetracker.h"

//minimum sleep delay (between refreshes / hammer attempts) in ms
#define SLEEPDELAY 150
#define IDLETIME 60000

extern GlobalContext * global;

class SiteThread {
  private:
    PotentialTracker * ptrack;
    std::vector<FTPThread *> conns;
    std::vector<ConnStateTracker> connstatetracker;
    std::vector<SiteRace *> races;
    FTPThreadCom * ftpthreadcom;
    pthread_t thread;
    pthread_mutex_t slots;
    sem_t notifysem;
    sem_t * list_refresh;
    unsigned int max_slots_up;
    unsigned int max_slots_dn;
    unsigned int slots_dn;
    unsigned int slots_up;
    unsigned int available;
    unsigned int loggedin;
    std::list<SiteThreadRequest> requests;
    std::list<SiteThreadRequest> requestsinprogress;
    std::list<SiteThreadRequestReady> requestsready;
    int requestidcounter;
    Site * site;
    void activate();
    void handleConnection(int);
    void handleRequest(int);
    static void * run(void *);
  public:
    SiteThread(std::string);
    ~SiteThread();
    void runInstance();
    void addRace(Race *, std::string, std::string);
    Site * getSite();
    SiteRace * getRace(std::string);
    bool getDownloadThread(SiteRace *, std::string, FTPThread **);
    bool getUploadThread(SiteRace *, std::string, FTPThread **);
    bool getReadyThread(SiteRace *, FTPThread **);
    bool getReadyThread(SiteRace *, std::string, FTPThread **, bool, bool);
    void returnThread(FTPThread *);
    void setNumConnections(unsigned int);
    bool downloadSlotAvailable();
    bool uploadSlotAvailable();
    int getCurrDown();
    int getCurrUp();
    int getCurrLogins();
    void raceGlobalComplete();
    void raceLocalComplete(SiteRace *);
    void transferComplete(bool isdownload);
    bool getSlot(bool);
    int requestFileList(std::string);
    bool requestReady(int);
    FileList * getFileList(int);
    void finishRequest(int);
    void requestViewFile(std::string);
    void pushPotential(int, std::string, SiteThread *);
    bool potentialCheck(int);
    std::vector<FTPThread *> * getConns();
    FTPThread * getConn(int);
    std::string getStatus(int);
};
