#pragma once
#include <string>
#include <list>

class SiteThread;

class PotentialElement {
  private:
    SiteThread * site;
    int potential;
    int site_dnslots;
    std::string filename;
  public:
    PotentialElement();
    SiteThread * getSite();
    int getSiteDownloadSlots();
    int getPotential();
    std::string getFileName();
    void update(SiteThread *, int, int, std::string);
};
