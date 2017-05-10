#pragma once

#include <string>
#include <list>
#include <set>
#include <map>
#include <utility>

#include "core/eventreceiver.h"
#include "core/pointer.h"
#include "sizelocationtrack.h"
#include "transferstatuscallback.h"


#define RACE_UPDATE_INTERVAL 250

#define RACE_STATUS_RUNNING 133
#define RACE_STATUS_DONE 134
#define RACE_STATUS_ABORTED 135
#define RACE_STATUS_TIMEOUT 136

enum SpreadProfile {
  SPREAD_RACE,
  SPREAD_DISTRIBUTE,
  SPREAD_PREPARE
};

class SiteRace;
class FileList;
class File;
class SiteLogic;
class TransferStatus;

typedef std::pair<std::string, std::pair<FileList *, FileList *> > FailedTransfer;

class Race : public EventReceiver, public TransferStatusCallback {
  private:
    void recalculateBestUnknownFileSizeEstimate();
    void setDone();
    void calculatePercentages();
    void calculateTotalFileSize();
    void addTransferAttempt(const Pointer<TransferStatus> &);
    std::string name;
    std::string group;
    std::string section;
    std::list<std::pair<SiteRace *, Pointer<SiteLogic> > > sites;
    std::map<SiteRace *, std::map<std::string, unsigned int> > sizes;
    std::list<SiteRace *> semidonesites;
    std::list<SiteRace *> donesites;
    unsigned int maxnumfilessiteprogress;
    std::map<std::string, std::list<SiteRace *> > sfvreports;
    std::map<std::string, unsigned int> estimatedsize;
    std::map<std::string, unsigned long long int> estimatedfilesizes;
    unsigned long long int bestunknownfilesizeestimate;
    std::map<std::string, std::list<SiteRace *> > subpathoccurences;
    std::list<std::string> estimatedsubpaths;
    std::map<std::string, std::map<std::string, unsigned long long int> > guessedfilelists;
    std::map<std::string, unsigned long long int> guessedfileliststotalfilesize;
    unsigned long long int guessedtotalfilesize;
    std::map<std::string, std::map<std::string, SizeLocationTrack> > sizelocationtrackers;
    std::map<FailedTransfer, int> transferattempts;
    int checkcount;
    std::string timestamp;
    unsigned int timespent;
    int status;
    unsigned int worst;
    unsigned int avg;
    unsigned int best;
    bool transferattemptscleared;
    unsigned int id;
    SpreadProfile profile;
  public:
    Race(unsigned int, SpreadProfile, const std::string &, const std::string &);
    ~Race();
    void addSite(SiteRace *, const Pointer<SiteLogic> &);
    void removeSite(SiteRace *);
    void removeSite(const Pointer<SiteLogic> &);
    std::list<std::pair<SiteRace *, Pointer<SiteLogic> > >::const_iterator begin() const;
    std::list<std::pair<SiteRace *, Pointer<SiteLogic> > >::const_iterator end() const;
    std::string getName() const;
    std::string getGroup() const;
    std::string getSection() const;
    bool sizeEstimated(const std::string &) const;
    unsigned int estimatedSize(const std::string &) const;
    unsigned int guessedSize(const std::string &) const;
    unsigned long long int estimatedTotalSize() const;
    unsigned long long int guessedFileSize(const std::string &, const std::string &) const;
    std::map<std::string, unsigned long long int>::const_iterator guessedFileListBegin(const std::string &) const;
    std::map<std::string, unsigned long long int>::const_iterator guessedFileListEnd(const std::string &) const;
    bool SFVReported(const std::string &) const;
    std::list<std::string> getSubPaths() const;
    int numSitesDone() const;
    int numSites() const;
    void updateSiteProgress(unsigned int);
    unsigned int getMaxSiteNumFilesProgress() const;
    bool isDone() const;
    std::string getTimeStamp() const;
    unsigned int getTimeSpent() const;
    std::string getSiteListText() const;
    SiteRace * getSiteRace(const std::string &) const;
    int getStatus() const;
    unsigned int getId() const;
    SpreadProfile getProfile() const;
    void reportNewSubDir(SiteRace *, const std::string &);
    void reportSFV(SiteRace *, const std::string &);
    void reportDone(SiteRace *);
    void reportSemiDone(SiteRace *);
    void reportSize(SiteRace *, FileList *, const std::string &, const std::set<std::string> &, bool);
    void setUndone();
    void reset();
    void abort();
    void setTimeout();
    int checksSinceLastUpdate();
    void resetUpdateCheckCounter();
    void tick(int);
    unsigned int getWorstCompletionPercentage() const;
    unsigned int getAverageCompletionPercentage() const;
    unsigned int getBestCompletionPercentage() const;
    bool hasFailedTransfer(File *, FileList *, FileList *) const;
    bool failedTransfersCleared() const;
    void addTransfer(const Pointer<TransferStatus> &);
    bool clearTransferAttempts();
    void transferSuccessful(const Pointer<TransferStatus> &);
    void transferFailed(const Pointer<TransferStatus> &, int);
};
