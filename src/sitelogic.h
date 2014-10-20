#pragma once

#include <string>
#include <vector>
#include <list>

#include "eventreceiver.h"

class TransferMonitor;
class ConnStateTracker;
class FTPConn;
class RawBuffer;
class SiteRace;
class FileList;
class Site;
class Race;
class GlobalContext;
class SiteLogicRequest;
class SiteLogicRequestReady;
class PotentialTracker;
class FileStore;
class TransferJob;

//minimum sleep delay (between refreshes / hammer attempts) in ms
#define SLEEPDELAY 150
//maximum number of dir refreshes in a row in the same race
#define MAXCHECKSROW 5

#define REQ_FILELIST 2620
#define REQ_RAW 2621
#define REQ_WIPE_RECURSIVE 2622
#define REQ_WIPE 2623
#define REQ_DEL_RECURSIVE 2624
#define REQ_DEL 2625
#define REQ_NUKE 2626

extern GlobalContext * global;

class SiteLogic : public EventReceiver {
  private:
    PotentialTracker * ptrack;
    std::vector<FTPConn *> conns;
    std::vector<ConnStateTracker> connstatetracker;
    std::vector<SiteRace *> races;
    std::list<SiteRace *> recentlylistedraces;
    std::list<TransferJob *> transferjobs;
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
    void handleConnection(int, bool);
    bool handleRequest(int);
    void handleRecursiveLogic(int);
    void handleRecursiveLogic(int, FileList *);
    void addRecentList(SiteRace *);
    bool wasRecentlyListed(SiteRace *) const;
    void refreshChangePath(int, SiteRace *, bool);
    void initTransfer(int);
    void handleFail(int);
    void handleTransferFail(int, int);
    void handleTransferFail(int, int, int);
    void reportTransferErrorAndFinish(int, int);
    void reportTransferErrorAndFinish(int, int, int);
    void getFileListConn(int);
    void getFileListConn(int, bool);
    void getFileListConn(int, SiteRace *, FileList *);
    void passiveModeCommand(int);
    static void * run(void *);
    bool poke;
  public:
    SiteLogic(std::string);
    ~SiteLogic();
    void runInstance();
    void addRace(Race *, std::string, std::string);
    void addTransferJob(TransferJob *);
    void tick(int);
    void connectFailed(int);
    void userDenied(int);
    void userDeniedSiteFull(int);
    void userDeniedSimultaneousLogins(int);
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
    Site * getSite() const;
    SiteRace * getRace(std::string) const;
    void lockConnList(int);
    bool lockDownloadConn(std::string, std::string, int *);
    bool lockUploadConn(std::string, std::string, int *);
    bool getReadyConn(std::string, int *);
    bool getReadyConn(std::string, std::string, int *, bool, bool);
    void returnConn(int);
    void setNumConnections(unsigned int);
    bool downloadSlotAvailable() const;
    bool uploadSlotAvailable() const;
    int getCurrDown() const;
    int getCurrUp() const;
    int getCurrLogins() const;
    void activate();
    void connectConn(int);
    void disconnectConn(int);
    void listCompleted(int, int);
    void issueRawCommand(unsigned int, std::string);
    RawBuffer * getRawCommandBuffer() const;
    void raceGlobalComplete();
    void raceLocalComplete(SiteRace *);
    void transferComplete(bool isdownload);
    bool getSlot(bool);
    int requestFileList(std::string);
    int requestRawCommand(std::string);
    int requestWipe(std::string, bool);
    int requestDelete(std::string, bool);
    int requestNuke(std::string, int, std::string);
    bool requestReady(int) const;
    void abortRace(std::string);
    FileList * getFileList(int) const;
    std::string getRawCommandResult(int);
    bool finishRequest(int);
    void pushPotential(int, std::string, SiteLogic *);
    bool potentialCheck(int);
    void updateName();
    const std::vector<FTPConn *> * getConns() const;
    FTPConn * getConn(int) const;
    std::string getStatus(int) const;
    void preparePassiveDownload(int, TransferMonitor *, std::string, std::string, bool, bool);
    void preparePassiveUpload(int, TransferMonitor *, std::string, std::string, bool, bool);
    void preparePassiveList(int, TransferMonitor *, bool);
    void download(int);
    void upload(int);
    void list(int);
    void listAll(int);
    void prepareActiveUpload(int, TransferMonitor *, std::string, std::string, std::string, bool);
    void prepareActiveDownload(int, TransferMonitor *, std::string, std::string, std::string, bool);
    void abortTransfer(int);
    const ConnStateTracker * getConnStateTracker(int) const;
};
