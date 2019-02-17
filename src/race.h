#pragma once

#include <memory>
#include <set>
#include <string>
#include <unordered_set>
#include <unordered_map>
#include <utility>

#include "core/eventreceiver.h"
#include "sizelocationtrack.h"
#include "transferstatuscallback.h"


#define RACE_UPDATE_INTERVAL 250

enum RaceStatus {
  RACE_STATUS_RUNNING,
  RACE_STATUS_DONE,
  RACE_STATUS_ABORTED,
  RACE_STATUS_TIMEOUT
};

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
class SkipList;

typedef std::pair<std::string, std::pair<FileList *, FileList *> > FailedTransfer;

struct FailedTransferHash {
public:
  std::size_t operator()(const std::pair<std::string, std::pair<FileList *, FileList *> > & x) const
  {
    return std::hash<std::string>()(x.first) + std::hash<FileList *>()(x.second.first) + std::hash<FileList *>()(x.second.second);
  }
};

struct SitesComparator {
  bool operator()(const std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > & a,
                  const std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > & b) const;
};

class Race : public EventReceiver, public TransferStatusCallback {
  private:
    void recalculateBestUnknownFileSizeEstimate();
    void setDone();
    void calculatePercentages();
    void calculateTotal();
    void addTransferAttempt(const std::shared_ptr<TransferStatus> &);
    std::string name;
    std::string group;
    std::string section;
    const SkipList & sectionskiplist;
    std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> >, SitesComparator> sites;
    std::unordered_map<std::shared_ptr<SiteRace>, std::unordered_map<std::string, unsigned int> > sizes;
    std::unordered_set<std::shared_ptr<SiteRace>> semidonesites;
    std::unordered_set<std::shared_ptr<SiteRace>> donesites;
    unsigned int maxnumfilessiteprogress;
    std::unordered_map<std::string, std::unordered_set<std::shared_ptr<SiteRace>> > sfvreports;
    std::unordered_map<std::string, unsigned int> estimatedsize;
    std::unordered_map<std::string, unsigned long long int> estimatedfilesizes;
    unsigned long long int bestunknownfilesizeestimate;
    std::unordered_map<std::string, std::unordered_set<std::shared_ptr<SiteRace>> > subpathoccurences;
    std::unordered_set<std::string> estimatedsubpaths;
    std::unordered_map<std::string, std::unordered_map<std::string, unsigned long long int> > guessedfilelists;
    std::unordered_map<std::string, unsigned long long int> guessedfileliststotalfilesize;
    unsigned long long int guessedtotalfilesize;
    unsigned int guessedtotalnumfiles;
    std::unordered_map<std::string, std::unordered_map<std::string, SizeLocationTrack> > sizelocationtrackers;
    std::unordered_map<FailedTransfer, int, FailedTransferHash> transferattempts;
    int checkcount;
    std::string timestamp;
    unsigned int timespent;
    RaceStatus status;
    unsigned int worst;
    unsigned int avg;
    unsigned int best;
    bool transferattemptscleared;
    unsigned int id;
    SpreadProfile profile;
    unsigned long long int transferredsize;
    unsigned int transferredfiles;
  public:
    Race(unsigned int, SpreadProfile, const std::string &, const std::string &);
    ~Race();
    CallbackType callbackType() const override;
    void addSite(const std::shared_ptr<SiteRace> & sr, const std::shared_ptr<SiteLogic> &);
    void removeSite(const std::shared_ptr<SiteRace> & sr);
    void removeSite(const std::shared_ptr<SiteLogic> &);
    std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator begin() const;
    std::set<std::pair<std::shared_ptr<SiteRace>, std::shared_ptr<SiteLogic> > >::const_iterator end() const;
    std::string getName() const;
    std::string getGroup() const;
    std::string getSection() const;
    bool sizeEstimated(const std::string &) const;
    unsigned int estimatedSize(const std::string &) const;
    unsigned int guessedSize(const std::string &) const;
    unsigned long long int estimatedTotalSize() const;
    unsigned long long int guessedFileSize(const std::string &, const std::string &) const;
    std::unordered_map<std::string, unsigned long long int>::const_iterator guessedFileListBegin(const std::string &) const;
    std::unordered_map<std::string, unsigned long long int>::const_iterator guessedFileListEnd(const std::string &) const;
    bool SFVReported(const std::string &) const;
    std::unordered_set<std::string> getSubPaths() const;
    int numSitesDone() const;
    int numSites() const;
    void updateSiteProgress(unsigned int);
    unsigned int getMaxSiteNumFilesProgress() const;
    bool isDone() const;
    std::string getTimeStamp() const;
    unsigned int getTimeSpent() const;
    std::string getSiteListText() const;
    std::shared_ptr<SiteRace> getSiteRace(const std::string & site) const;
    RaceStatus getStatus() const;
    unsigned int getId() const;
    SpreadProfile getProfile() const;
    unsigned long long int getTransferredSize() const;
    unsigned int getTransferredFiles() const;
    void reportNewSubDir(const std::shared_ptr<SiteRace> & sr, const std::string &);
    void reportSFV(const std::shared_ptr<SiteRace> & sr, const std::string &);
    void reportDone(const std::shared_ptr<SiteRace> & sr);
    void reportSemiDone(const std::shared_ptr<SiteRace> & sr);
    void reportSize(const std::shared_ptr<SiteRace> & sr, FileList *, const std::string &, const std::unordered_set<std::string> &, bool);
    void setUndone();
    void reset();
    void abort();
    void setTimeout();
    int timeoutCheck();
    void resetUpdateCheckCounter();
    void tick(int);
    unsigned int getWorstCompletionPercentage() const;
    unsigned int getAverageCompletionPercentage() const;
    unsigned int getBestCompletionPercentage() const;
    bool hasFailedTransfer(const std::string & filename, FileList * fls, FileList * fld) const;
    bool failedTransfersCleared() const;
    const SkipList & getSectionSkipList() const;
    void addTransfer(const std::shared_ptr<TransferStatus> &);
    bool clearTransferAttempts(bool clearstate = true);
    void transferSuccessful(const std::shared_ptr<TransferStatus> &);
    void transferFailed(const std::shared_ptr<TransferStatus> &, int);
    void addTransferStatsFile(unsigned long long int);
};
