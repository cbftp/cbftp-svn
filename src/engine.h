#pragma once

#include <string>
#include <list>
#include <map>

#include "eventreceiver.h"
#include "pointer.h"

#define SPREAD 0
#define POKEINTERVAL 1000
#define MAXCHECKSTIMEOUT 120
#define STATICTIMEFORCOMPLETION 5000
#define DIROBSERVETIME 20000
#define SFVDIROBSERVETIME 5000

class GlobalContext;
class Race;
class TransferJob;
class SiteRace;
class FileList;
class File;
class ScoreBoard;
class SiteLogic;
class PendingTransfer;
class Site;

extern GlobalContext * global;

class Engine : public EventReceiver {
  private:
    std::list<Pointer<Race> > allraces;
    std::list<Pointer<Race> > currentraces;
    std::list<Pointer<TransferJob>  > alltransferjobs;
    std::list<Pointer<TransferJob> > currenttransferjobs;
    Pointer<ScoreBoard> scoreboard;
    std::map<Pointer<TransferJob>, std::list<PendingTransfer> > pendingtransfers;
    int maxavgspeed;
    void estimateRaceSizes();
    void reportCurrentSize(SiteRace *, FileList *, bool final);
    void refreshScoreBoard();
    void issueOptimalTransfers();
    void setSpeedScale();
    unsigned short calculateScore(File *, Pointer<Race>, FileList *, SiteRace *, FileList *, SiteRace *, int, bool *, bool) const;
    void checkIfRaceComplete(SiteLogic *, Pointer<Race>);
    void raceComplete(Pointer<Race>);
    void transferJobComplete(Pointer<TransferJob>);
    void issueGlobalComplete(Pointer<Race>);
    void refreshPendingTransferList(Pointer<TransferJob>);
    void checkStartPoke();
    void addPendingTransfer(std::list<PendingTransfer> &, PendingTransfer &);
    Pointer<Race> getCurrentRace(std::string) const;
    bool checkBannedGroup(Site *, const std::string &);
    bool pokeregistered;
    unsigned int dropped;
  public:
    Engine();
    ~Engine();
    bool newRace(std::string, std::string, std::list<std::string>);
    void newTransferJobDownload(std::string, std::string, FileList *, std::string);
    void newTransferJobDownload(std::string, std::string, FileList *, std::string, std::string);
    void newTransferJobUpload(std::string, std::string, std::string, FileList *);
    void newTransferJobUpload(std::string, std::string, std::string, std::string, FileList *);
    void newTransferJobFXP(std::string, FileList *, std::string, FileList *, std::string);
    void newTransferJobFXP(std::string, std::string, FileList *, std::string, std::string, FileList *);
    void removeSiteFromRace(std::string, std::string);
    void abortRace(std::string);
    void deleteOnAllSites(std::string);
    void abortTransferJob(Pointer<TransferJob>);
    void raceFileListRefreshed(SiteLogic *, Pointer<Race>);
    bool transferJobActionRequest(Pointer<TransferJob>);
    int currentRaces() const;
    int allRaces() const;
    int currentTransferJobs() const;
    int allTransferJobs() const;
    Pointer<Race> getRace(std::string) const;
    Pointer<TransferJob> getTransferJob(std::string) const;
    std::list<Pointer<Race> >::const_iterator getRacesBegin() const;
    std::list<Pointer<Race> >::const_iterator getRacesEnd() const;
    std::list<Pointer<TransferJob> >::const_iterator getTransferJobsBegin() const;
    std::list<Pointer<TransferJob> >::const_iterator getTransferJobsEnd() const;
    void tick(int);
    void addSiteToRace(Pointer<Race>, std::string);

    Pointer<ScoreBoard> getScoreBoard() const;
};
