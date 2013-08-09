#pragma once

#include <string>
#include <vector>
#include <list>

#include "sitemanager.h"
#include "ftpconn.h"
#include "filelist.h"
#include "siterace.h"
#include "globalcontext.h"
#include "scoreboardelement.h"
#include "potentialtracker.h"
#include "sitelogicrequest.h"
#include "sitelogicrequestready.h"
#include "ui/uicommunicator.h"
#include "tickpoke.h"
#include "connstatetracker.h"
#include "rawbuffer.h"
#include "eventreceiver.h"
#include "sitelogicbase.h"
#include "enginebase.h"
#include "transfermonitorbase.h"

//minimum sleep delay (between refreshes / hammer attempts) in ms
#define SLEEPDELAY 150
//maximum number of dir refreshes in a row in the same race
#define MAXCHECKSROW 5

extern GlobalContext * global;

class SiteLogic : public SiteLogicBase {
  private:
    PotentialTracker * ptrack;
    std::vector<FTPConn *> conns;
    std::vector<ConnStateTracker> connstatetracker;
    std::vector<SiteRace *> races;
    std::list<SiteRace *> recentlylistedraces;
    RawBuffer * rawbuf;
    unsigned int max_slots_up;
    unsigned int max_slots_dn;
    unsigned int slots_dn;
    unsigned int slots_up;
    unsigned int available;
    unsigned int loggedin;
    unsigned int wantedloggedin;
    std::list<SiteLogicRequest> requests;
    std::list<SiteLogicRequest> requestsinprogress;
    std::list<SiteLogicRequestReady> requestsready;
    int requestidcounter;
    Site * site;
    void activate();
    void handleConnection(int, bool);
    void handleRequest(int);
    void addRecentList(SiteRace *);
    bool wasRecentlyListed(SiteRace *);
    void refreshChangePath(int, SiteRace *, bool);
    void initTransfer(int);
    void handleTransferFail(int, bool, int);
    static void * run(void *);
    bool poke;
  public:
    SiteLogic(std::string);
    ~SiteLogic();
    void runInstance();
    void addRace(Race *, std::string, std::string);
    void tick(int);
    void connectFailed(int);
    void userDenied(int);
    void loginKillFailed(int);
    void passDenied(int);
    void TLSFailed(int);
    void listRefreshed(int);
    void unexpectedResponse(int);
    void commandSuccess(int);
    void commandFail(int);
    void gotPath(int, std::string);
    void rawCommandResultRetrieved(int, std::string);
    void gotPassiveAddress(int, std::string);
    void timedout(int);
    void disconnected(int);
    void requestSelect();
    Site * getSite();
    SiteRace * getRace(std::string);
    bool lockDownloadConn(FileList *, std::string, int *);
    bool lockUploadConn(FileList *, std::string, int *);
    bool getReadyConn(FileList *, int *);
    bool getReadyConn(FileList *, std::string, int *, bool, bool);
    void returnConn(int);
    void setNumConnections(unsigned int);
    bool downloadSlotAvailable();
    bool uploadSlotAvailable();
    int getCurrDown();
    int getCurrUp();
    int getCurrLogins();
    void connectConn(int);
    void disconnectConn(int);
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
    void pushPotential(int, std::string, SiteLogic *);
    bool potentialCheck(int);
    void updateName();
    std::vector<FTPConn *> * getConns();
    FTPConn * getConn(int);
    std::string getStatus(int);
    void preparePassiveDownload(int, TransferMonitorBase *, FileList *, std::string, bool);
    void preparePassiveUpload(int, TransferMonitorBase *, FileList *, std::string, bool);
    void passiveDownload(int);
    void passiveUpload(int);
    void activeUpload(int, TransferMonitorBase *, FileList *, std::string, std::string, bool);
    void activeDownload(int, TransferMonitorBase *, FileList *, std::string, std::string, bool);
    void abortTransfer(int);
};
