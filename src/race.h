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
    std::map<SiteRace *, unsigned int> sizes;
    std::list<SiteRace *> donesites;
    int maxfilelistsize;
    unsigned int estimatedsize;
    bool sizeestimated;
    bool done;
  public:
    Race(std::string, std::string);
    void addSite(SiteThread *);
    std::list<SiteThread *>::iterator begin();
    std::list<SiteThread *>::iterator end();
    std::string getName();
    std::string getSection();
    bool sizeEstimated();
    int estimatedSize();
    int numSites();
    void updateSiteProgress(int);
    int getMaxSiteProgress();
    bool isDone();
    void reportDone(SiteRace *);
    void reportSize(SiteRace *, unsigned int);
};
