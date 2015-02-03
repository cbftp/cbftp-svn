#pragma once

#include <string>
#include <list>
#include <map>

#include "eventreceiver.h"

#define SPREAD 0
#define POKEINTERVAL 1000
#define MAXCHECKSTIMEOUT 120
#define STATICTIMEFORCOMPLETION 5000

class GlobalContext;
class Race;
class TransferJob;
class SiteRace;
class FileList;
class File;
class ScoreBoard;
class SiteLogic;
class PendingTransfer;

extern GlobalContext * global;

class Engine : public EventReceiver {
  private:
    std::list<Race *> allraces;
    std::list<Race *> currentraces;
    std::list<TransferJob *> alltransferjobs;
    std::list<TransferJob *> currenttransferjobs;
    ScoreBoard * scoreboard;
    std::map<TransferJob *, std::list<PendingTransfer> > pendingtransfers;
    int maxavgspeed;
    void estimateRaceSizes();
    void reportCurrentSize(SiteRace *, FileList *, bool final);
    void refreshScoreBoard();
    void issueOptimalTransfers();
    void setSpeedScale();
    int calculateScore(File *, Race *, FileList *, SiteRace *, FileList *, SiteRace *, int, bool *, bool) const;
    void checkIfRaceComplete(SiteLogic *, Race *);
    void raceComplete(Race *);
    void transferJobComplete(TransferJob *);
    void issueGlobalComplete(Race *);
    void refreshPendingTransferList(TransferJob *);
    void checkStartPoke();
    bool pokeregistered;
    unsigned int dropped;
  public:
    Engine();
    void newRace(std::string, std::string, std::list<std::string>);
    void newTransferJobDownload(std::string, std::string, FileList *, std::string);
    void newTransferJobDownload(std::string, std::string, FileList *, std::string, std::string);
    void newTransferJobUpload(std::string, std::string, std::string, FileList *);
    void newTransferJobUpload(std::string, std::string, std::string, std::string, FileList *);
    void newTransferJobFXP(std::string, FileList *, std::string, FileList *, std::string);
    void newTransferJobFXP(std::string, std::string, FileList *, std::string, std::string, FileList *);
    void removeSiteFromRace(std::string, std::string);
    void abortRace(std::string);
    void raceFileListRefreshed(SiteLogic *, Race *);
    void transferJobActionRequest(TransferJob *);
    int currentRaces() const;
    int allRaces() const;
    int currentTransferJobs() const;
    int allTransferJobs() const;
    Race * getRace(std::string) const;
    TransferJob * getTransferJob(std::string) const;
    std::list<Race *>::iterator getRacesIteratorBegin();
    std::list<Race *>::iterator getRacesIteratorEnd();
    std::list<Race *>::const_iterator getRacesIteratorBegin() const;
    std::list<Race *>::const_iterator getRacesIteratorEnd() const;
    std::list<TransferJob *>::const_iterator getTransferJobsIteratorBegin() const;
    std::list<TransferJob *>::const_iterator getTransferJobsIteratorEnd() const;
    void tick(int);

    ScoreBoard * getScoreBoard() const;
};
