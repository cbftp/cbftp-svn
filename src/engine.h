#pragma once

#include <string>
#include <list>
#include <map>

#include "eventreceiver.h"
#include "pointer.h"

#define SPREAD 0
#define POKEINTERVAL 1000
#define MAXCHECKSTIMEOUT 60
#define STATICTIMEFORCOMPLETION 5000
#define DIROBSERVETIME 20000
#define SFVDIROBSERVETIME 5000

class Race;
class TransferJob;
class SiteRace;
class FileList;
class File;
class ScoreBoard;
class SiteLogic;
class PendingTransfer;
class Site;
class PreparedRace;

class Engine : public EventReceiver {
  private:
    std::list<Pointer<Race> > allraces;
    std::list<Pointer<Race> > currentraces;
    std::list<Pointer<PreparedRace> > preparedraces;
    std::list<Pointer<TransferJob>  > alltransferjobs;
    std::list<Pointer<TransferJob> > currenttransferjobs;
    Pointer<ScoreBoard> scoreboard;
    std::map<Pointer<TransferJob>, std::list<PendingTransfer> > pendingtransfers;
    int maxavgspeed;
    bool newSpreadJob(int, const std::string &, const std::string &, const std::list<std::string> &);
    void estimateRaceSizes();
    void reportCurrentSize(SiteRace *, FileList *, bool final);
    void refreshScoreBoard();
    void issueOptimalTransfers();
    void setSpeedScale();
    unsigned short calculateScore(File *, Pointer<Race> &, FileList *, SiteRace *, FileList *, SiteRace *, int, bool *, bool) const;
    void checkIfRaceComplete(SiteLogic *, Pointer<Race> &);
    void raceComplete(Pointer<Race> &);
    void transferJobComplete(Pointer<TransferJob> &);
    void issueGlobalComplete(Pointer<Race> &);
    void refreshPendingTransferList(Pointer<TransferJob> &);
    void checkStartPoke();
    void addPendingTransfer(std::list<PendingTransfer> &, PendingTransfer &);
    Pointer<Race> getCurrentRace(const std::string &) const;
    bool checkBannedGroup(Site *, const std::string &);
    bool pokeregistered;
    unsigned int dropped;
    unsigned int nextid;
  public:
    Engine();
    ~Engine();
    bool newRace(const std::string &, const std::string &, const std::list<std::string> &);
    void prepareRace(const std::string &, const std::string &, const std::list<std::string> &);
    bool newDistribute(const std::string &, const std::string &, const std::list<std::string> &);
    void startPreparedRace(unsigned int);
    void deletePreparedRace(unsigned int);
    void startLatestPreparedRace();
    void newTransferJobDownload(std::string, std::string, FileList *, std::string);
    void newTransferJobDownload(std::string, std::string, FileList *, std::string, std::string);
    void newTransferJobUpload(std::string, std::string, std::string, FileList *);
    void newTransferJobUpload(std::string, std::string, std::string, std::string, FileList *);
    void newTransferJobFXP(std::string, FileList *, std::string, FileList *, std::string);
    void newTransferJobFXP(std::string, std::string, FileList *, std::string, std::string, FileList *);
    void removeSiteFromRace(Pointer<Race> &, const std::string &);
    void abortRace(Pointer<Race> &);
    void deleteOnAllSites(Pointer<Race> &);
    void abortTransferJob(Pointer<TransferJob> &);
    void raceFileListRefreshed(SiteLogic *, SiteRace *);
    bool transferJobActionRequest(Pointer<TransferJob> &);
    int preparedRaces() const;
    int currentRaces() const;
    int allRaces() const;
    int currentTransferJobs() const;
    int allTransferJobs() const;
    Pointer<Race> getRace(unsigned int) const;
    Pointer<TransferJob> getTransferJob(unsigned int) const;
    std::list<Pointer<PreparedRace> >::const_iterator getPreparedRacesBegin() const;
    std::list<Pointer<PreparedRace> >::const_iterator getPreparedRacesEnd() const;
    std::list<Pointer<Race> >::const_iterator getRacesBegin() const;
    std::list<Pointer<Race> >::const_iterator getRacesEnd() const;
    std::list<Pointer<TransferJob> >::const_iterator getTransferJobsBegin() const;
    std::list<Pointer<TransferJob> >::const_iterator getTransferJobsEnd() const;
    void tick(int);
    void addSiteToRace(Pointer<Race> &, const std::string &);
    unsigned int getLatestId() const;

    Pointer<ScoreBoard> getScoreBoard() const;
};
