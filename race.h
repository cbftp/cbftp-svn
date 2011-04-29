#pragma once
#include <string>
#include <list>

class SiteThread;

class Race {
  private:
    std::string name;
    std::string section;
    std::list<SiteThread *> sites;
    int maxfilelistsize;
  public:
    Race(std::string, std::string);
    void addSite(SiteThread *);
    std::list<SiteThread *>::iterator begin();
    std::list<SiteThread *>::iterator end();
    std::string getName();
    void updateSiteProgress(int);
    int getMaxSiteProgress();
};
