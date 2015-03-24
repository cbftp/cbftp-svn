#pragma once

#include <string>
#include <list>
#include <map>
#include <utility>

#include "sizelocationtrack.h"
#include "eventreceiver.h"

#define RACE_UPDATE_INTERVAL 250

#define RACE_STATUS_RUNNING 133
#define RACE_STATUS_DONE 134
#define RACE_STATUS_ABORTED 135
#define RACE_STATUS_TIMEOUT 136

class SiteRace;
class FileList;
class SiteLogic;

class Race : public EventReceiver {
  private:
    void recalculateBestUnknownFileSizeEstimate();
    void setDone();
    void calculatePercentages();
    void calculateTotalFileSize();
    std::string name;
    std::string group;
    std::string section;
    std::list<std::pair<SiteRace *, SiteLogic *> > sites;
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
    int checkcount;
    std::string timestamp;
    unsigned int timespent;
    int status;
    unsigned int worst;
    unsigned int avg;
    unsigned int best;
  public:
    Race(std::string, std::string);
    ~Race();
    void addSite(SiteRace *, SiteLogic *);
    void removeSite(SiteRace *);
    void removeSite(SiteLogic *);
    std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator begin() const;
    std::list<std::pair<SiteRace *, SiteLogic *> >::const_iterator end() const;
    std::string getName() const;
    std::string getGroup() const;
    std::string getSection() const;
    bool sizeEstimated(std::string) const;
    unsigned int estimatedSize(std::string) const;
    unsigned int guessedSize(std::string) const;
    unsigned long long int estimatedTotalSize() const;
    unsigned long long int guessedFileSize(std::string, std::string) const;
    std::map<std::string, unsigned long long int>::const_iterator guessedFileListBegin(std::string) const;
    std::map<std::string, unsigned long long int>::const_iterator guessedFileListEnd(std::string) const;
    bool SFVReported(std::string) const;
    std::list<std::string> getSubPaths() const;
    int numSitesDone() const;
    int numSites() const;
    void updateSiteProgress(unsigned int);
    unsigned int getMaxSiteNumFilesProgress() const;
    bool isDone() const;
    std::string getTimeStamp() const;
    unsigned int getTimeSpent() const;
    std::string getSiteListText() const;
    SiteRace * getSiteRace(std::string) const;
    int getStatus() const;
    void reportNewSubDir(SiteRace *, std::string);
    void reportSFV(SiteRace *, std::string);
    void reportDone(SiteRace *);
    void reportSemiDone(SiteRace *);
    void reportSize(SiteRace *, FileList *, std::string, std::list<std::string> *, bool);
    void setUndone();
    void abort();
    void setTimeout();
    int checksSinceLastUpdate();
    void resetUpdateCheckCounter();
    void tick(int);
    unsigned int getWorstCompletionPercentage() const;
    unsigned int getAverageCompletionPercentage() const;
    unsigned int getBestCompletionPercentage() const;
};
