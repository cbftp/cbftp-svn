#pragma once

#include <string>

class SiteLogic;

class PotentialElement {
  private:
    SiteLogic * site;
    int potential;
    int site_dnslots;
    std::string filename;
  public:
    PotentialElement();
    SiteLogic * getSite();
    int getSiteDownloadSlots();
    int getPotential();
    std::string getFileName();
    void update(SiteLogic *, int, int, std::string);
};
