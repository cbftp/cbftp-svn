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
    std::string name;
    std::string group;
    std::string section;
    std::list<SiteLogic *> sites;
    std::map<SiteRace *, std::map<std::string, unsigned int> > sizes;
    std::list<SiteRace *> donesites;
    int maxfilelistsize;
    std::map<std::string, std::list<SiteRace *> > sfvreports;
    std::map<std::string, unsigned int> guessedsize;
    std::map<std::string, unsigned int> estimatedsize;
    std::map<std::string, std::list<SiteRace *> > subpathoccurences;
    std::list<std::string> estimatedsubpaths;
    std::list<std::string> guessedfilelist;
    std::map<std::string, std::map<std::string, SizeLocationTrack> > sizelocationtrackers;
    bool done;
  public:
    Race(std::string, std::string);
    void addSite(SiteLogic *);
    void removeSite(SiteLogic *);
    std::list<SiteLogic *>::iterator begin();
    std::list<SiteLogic *>::iterator end();
    std::string getName();
    std::string getGroup();
    std::string getSection();
    bool sizeEstimated(std::string);
    unsigned int estimatedSize(std::string);
    unsigned int guessedSize(std::string);
    void prepareGuessedFileList(std::string subpath);
    std::list<std::string>::iterator guessedFileListBegin();
    std::list<std::string>::iterator guessedFileListEnd();
    unsigned long long int guessedFileSize(std::string, std::string);
    bool SFVReported(std::string);
    std::list<std::string> getSubPaths();
    int numSites();
    void updateSiteProgress(int);
    int getMaxSiteProgress();
    bool isDone();
    void reportNewSubDir(SiteRace *, std::string);
    void reportSFV(SiteRace *, std::string);
    void reportDone(SiteRace *);
    void reportSize(SiteRace *, FileList *, std::string, std::list<std::string> *, bool);
    void setUndone();
    void abort();
};
