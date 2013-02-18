#pragma once

#include <string>
#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream>

class SiteThread;
class SiteRace;

class Race {
  private:
    std::string name;
    std::string section;
    std::list<SiteThread *> sites;
    std::map<SiteRace *, std::map<std::string, unsigned int> > sizes;
    std::list<SiteRace *> donesites;
    int maxfilelistsize;
    std::map<std::string, std::list<SiteRace *> > sfvreports;
    std::map<std::string, unsigned int> guessedsize;
    std::map<std::string, unsigned int> estimatedsize;
    std::map<std::string, std::list<SiteRace *> > subpathoccurences;
    std::list<std::string> estimatedsubpaths;
    bool done;
  public:
    Race(std::string, std::string);
    void addSite(SiteThread *);
    std::list<SiteThread *>::iterator begin();
    std::list<SiteThread *>::iterator end();
    std::string getName();
    std::string getSection();
    bool sizeEstimated(std::string);
    unsigned int estimatedSize(std::string);
    unsigned int guessedSize(std::string);
    bool SFVReported(std::string);
    std::list<std::string> getSubPaths();
    int numSites();
    void updateSiteProgress(int);
    int getMaxSiteProgress();
    bool isDone();
    void reportNewSubDir(SiteRace *, std::string);
    void reportSFV(SiteRace *, std::string);
    void reportDone(SiteRace *);
    void reportSize(SiteRace *, std::string, unsigned int);
};
