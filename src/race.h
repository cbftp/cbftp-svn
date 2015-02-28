#pragma once

#include <string>
#include <list>
#include <map>

#include "sizelocationtrack.h"

class SiteLogic;
class SiteRace;
class FileList;

class Race {
  private:
    void recalculateBestUnknownFileSizeEstimate();
    std::string name;
    std::string group;
    std::string section;
    std::list<SiteLogic *> sites;
    std::map<SiteRace *, std::map<std::string, unsigned int> > sizes;
    std::list<SiteRace *> semidonesites;
    std::list<SiteRace *> donesites;
    int maxfilelistsize;
    std::map<std::string, std::list<SiteRace *> > sfvreports;
    std::map<std::string, unsigned int> guessedsize;
    std::map<std::string, unsigned int> estimatedsize;
    std::map<std::string, unsigned long long int> estimatedfilesizes;
    unsigned long long int bestunknownfilesizeestimate;
    std::map<std::string, std::list<SiteRace *> > subpathoccurences;
    std::list<std::string> estimatedsubpaths;
    std::list<std::string> guessedfilelist;
    std::map<std::string, std::map<std::string, SizeLocationTrack> > sizelocationtrackers;
    bool done;
    bool aborted;
    int checkcount;
  public:
    Race(std::string, std::string);
    void addSite(SiteLogic *);
    void removeSite(SiteLogic *);
    std::list<SiteLogic *>::const_iterator begin() const;
    std::list<SiteLogic *>::const_iterator end() const;
    std::string getName() const;
    std::string getGroup() const;
    std::string getSection() const;
    bool sizeEstimated(std::string) const;
    unsigned int estimatedSize(std::string) const;
    unsigned int guessedSize(std::string) const;
    void prepareGuessedFileList(std::string subpath);
    std::list<std::string>::const_iterator guessedFileListBegin() const;
    std::list<std::string>::const_iterator guessedFileListEnd() const;
    unsigned long long int guessedFileSize(std::string, std::string) const;
    bool SFVReported(std::string) const;
    std::list<std::string> getSubPaths() const;
    int numSites() const;
    void updateSiteProgress(int);
    int getMaxSiteProgress() const;
    bool isDone() const;
    bool isAborted() const;
    void reportNewSubDir(SiteRace *, std::string);
    void reportSFV(SiteRace *, std::string);
    void reportDone(SiteRace *);
    void reportSemiDone(SiteRace *);
    void reportSize(SiteRace *, FileList *, std::string, std::list<std::string> *, bool);
    void setUndone();
    void abort();
    int checksSinceLastUpdate();
    void resetUpdateCheckCounter();
};
