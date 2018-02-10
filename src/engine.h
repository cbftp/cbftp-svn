#pragma once

#include <string>
#include <list>
#include <map>
#include <set>

#include "core/eventreceiver.h"
#include "core/pointer.h"

class Race;
class TransferJob;
class SiteTransferJob;
class SiteRace;
class FileList;
class File;
class ScoreBoard;
class SiteLogic;
class PendingTransfer;
class Site;
class PreparedRace;
class Path;
class SkipList;

class Engine : public EventReceiver {
public:
  Engine();
  ~Engine();
  Pointer<Race> newRace(const std::string &, const std::string &, const std::list<std::string> &);
  Pointer<Race> newRace(const std::string &, const std::string &);
  void prepareRace(const std::string &, const std::string &, const std::list<std::string> &);
  void prepareRace(const std::string &, const std::string &);
  Pointer<Race> newDistribute(const std::string &, const std::string &, const std::list<std::string> &);
  Pointer<Race> newDistribute(const std::string &, const std::string &);
  void startPreparedRace(unsigned int);
  void deletePreparedRace(unsigned int);
  void startLatestPreparedRace();
  void toggleStartNextPreparedRace();
  void newTransferJobDownload(const std::string &, FileList *, const std::string &, const Path &);
  void newTransferJobDownload(const std::string &, FileList *, const std::string &, const Path &, const std::string &);
  void newTransferJobDownload(const std::string &, const Path &, const std::string &, const Path &, const std::string &);
  void newTransferJobUpload(const Path &, const std::string &, const std::string &, FileList *);
  void newTransferJobUpload(const Path &, const std::string &, const std::string &, FileList *, const std::string &);
  void newTransferJobUpload(const Path &, const std::string &, const std::string &, const Path &, const std::string &);
  void newTransferJobFXP(const std::string &, FileList *, const std::string &, FileList *, const std::string &);
  void newTransferJobFXP(const std::string &, FileList *, const std::string &, const std::string &, FileList *, const std::string &);
  void newTransferJobFXP(const std::string &, const Path &, const std::string &, const std::string &, const Path &, const std::string &);
  void removeSiteFromRace(Pointer<Race> &, const std::string &);
  void removeSiteFromRaceDeleteFiles(Pointer<Race> &, const std::string &, bool);
  void abortRace(Pointer<Race> &);
  void resetRace(Pointer<Race> &, bool);
  void deleteOnAllSites(Pointer<Race> &);
  void deleteOnSites(Pointer<Race> &, std::list<Pointer<Site> >);
  void deleteOnSites(Pointer<Race> &, std::list<Pointer<Site> >, bool);
  void abortTransferJob(Pointer<TransferJob> &);
  void raceFileListRefreshed(SiteLogic *, SiteRace *);
  void filelistUpdated();
  bool transferJobActionRequest(Pointer<SiteTransferJob> &);
  void raceActionRequest();
  void setPreparedRaceExpiryTime(int);
  void clearSkipListCaches();
  unsigned int preparedRaces() const;
  unsigned int currentRaces() const;
  unsigned int allRaces() const;
  unsigned int currentTransferJobs() const;
  unsigned int allTransferJobs() const;
  Pointer<Race> getRace(unsigned int) const;
  Pointer<Race> getRace(const std::string &) const;
  Pointer<TransferJob> getTransferJob(unsigned int) const;
  std::list<Pointer<PreparedRace> >::const_iterator getPreparedRacesBegin() const;
  std::list<Pointer<PreparedRace> >::const_iterator getPreparedRacesEnd() const;
  std::list<Pointer<Race> >::const_iterator getRacesBegin() const;
  std::list<Pointer<Race> >::const_iterator getRacesEnd() const;
  std::list<Pointer<TransferJob> >::const_iterator getTransferJobsBegin() const;
  std::list<Pointer<TransferJob> >::const_iterator getTransferJobsEnd() const;
  void tick(int);
  void addSiteToRace(Pointer<Race> &, const std::string &);
  Pointer<ScoreBoard> getScoreBoard() const;
  int getMaxPointsRaceTotal() const;
  int getMaxPointsFileSize() const;
  int getMaxPointsAvgSpeed() const;
  int getMaxPointsPriority() const;
  int getMaxPointsPercentageOwned() const;
  int getMaxPointsLowProgress() const;
  int getPriorityPoints(int) const;
  int getSpeedPoints(int) const;
  int getPreparedRaceExpiryTime() const;
  bool getNextPreparedRaceStarterEnabled() const;
  int getNextPreparedRaceStarterTimeRemaining() const;
 private:
  Pointer<Race> newSpreadJob(int, const std::string &, const std::string &, const std::list<std::string> &);
  Pointer<Race> newSpreadJob(int, const std::string &, const std::string &);
  void estimateRaceSizes();
  void estimateRaceSize(const Pointer<Race> &);
  void estimateRaceSize(const Pointer<Race> &, bool);
  void reportCurrentSize(const SkipList &, SiteRace *, FileList *, bool final);
  void refreshScoreBoard();
  void issueOptimalTransfers();
  void setSpeedScale();
  unsigned short calculateScore(File *, Pointer<Race> &, FileList *, SiteRace *, FileList *, SiteRace *, int, bool *, int prioritypoints, bool) const;
  void checkIfRaceComplete(SiteLogic *, Pointer<Race> &);
  void raceComplete(Pointer<Race>);
  void transferJobComplete(Pointer<TransferJob>);
  void issueGlobalComplete(Pointer<Race> &);
  void refreshPendingTransferList(Pointer<TransferJob> &);
  void checkStartPoke();
  void addPendingTransfer(std::list<PendingTransfer> &, PendingTransfer &);
  Pointer<Race> getCurrentRace(const std::string &) const;
  void preSeedPotentialData(Pointer<Race> &);
  bool raceTransferPossible(const Pointer<SiteLogic> &, const Pointer<SiteLogic> &, Pointer<Race> &) const;
  std::list<Pointer<Race> > allraces;
  std::list<Pointer<Race> > currentraces;
  std::list<Pointer<PreparedRace> > preparedraces;
  std::list<Pointer<TransferJob>  > alltransferjobs;
  std::list<Pointer<TransferJob> > currenttransferjobs;
  Pointer<ScoreBoard> scoreboard;
  std::map<Pointer<TransferJob>, std::list<PendingTransfer> > pendingtransfers;
  int maxavgspeed;
  bool pokeregistered;
  unsigned int dropped;
  unsigned int nextid;
  int maxpointsfilesize;
  int maxpointsavgspeed;
  int maxpointspriority;
  int maxpointspercentageowned;
  int maxpointslowprogress;
  int preparedraceexpirytime;
  bool startnextprepared;
  int nextpreparedtimeremaining;
};
