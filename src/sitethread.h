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
#include "rawbuffer.h"

//minimum sleep delay (between refreshes / hammer attempts) in ms
#define SLEEPDELAY 150
//maximum number of dir refreshes in a row in the same race
#define MAXCHECKSROW 5

extern GlobalContext * global;

class SiteThread {
  private:
    PotentialTracker * ptrack;
    std::vector<FTPThread *> conns;
    std::vector<ConnStateTracker> connstatetracker;
    std::vector<SiteRace *> races;
    std::list<SiteRace *> recentlylistedraces;
    FTPThreadCom * ftpthreadcom;
    RawBuffer * rawbuf;
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
    unsigned int wantedloggedin;
    std::list<SiteThreadRequest> requests;
    std::list<SiteThreadRequest> requestsinprogress;
    std::list<SiteThreadRequestReady> requestsready;
    int requestidcounter;
    Site * site;
    void activate();
    void handleConnection(int);
    void handleConnection(int, bool);
    void handleRequest(int);
    void addRecentList(SiteRace *);
    bool wasRecentlyListed(SiteRace *);
    static void * run(void *);
  public:
    SiteThread(std::string);
    ~SiteThread();
    void runInstance();
    void addRace(Race *, std::string, std::string);
    Site * getSite();
    SiteRace * getRace(std::string);
    bool getDownloadThread(FileList *, std::string, FTPThread **);
    bool getUploadThread(FileList *, std::string, FTPThread **);
    bool getReadyThread(FileList *, FTPThread **);
    bool getReadyThread(FileList *, std::string, FTPThread **, bool, bool);
    void returnThread(FTPThread *);
    void setNumConnections(unsigned int);
    bool downloadSlotAvailable();
    bool uploadSlotAvailable();
    int getCurrDown();
    int getCurrUp();
    int getCurrLogins();
    void connectThread(int);
    void disconnectThread(int);
    void issueRawCommand(unsigned int, std::string);
    RawBuffer * getRawCommandBuffer();
    void raceGlobalComplete();
    void raceLocalComplete(SiteRace *);
    void transferComplete(bool isdownload);
    bool getSlot(bool);
    int requestFileList(std::string);
    int requestRawCommand(std::string);
    bool requestReady(int);
    FileList * getFileList(int);
    std::string getRawCommandResult(int);
    void finishRequest(int);
    void requestViewFile(std::string);
    void pushPotential(int, std::string, SiteThread *);
    bool potentialCheck(int);
    std::vector<FTPThread *> * getConns();
    FTPThread * getConn(int);
    std::string getStatus(int);
};
