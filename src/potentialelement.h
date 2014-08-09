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
    SiteLogic * getSite() const;
    int getSiteDownloadSlots() const;
    int getPotential() const;
    std::string getFileName() const;
    void update(SiteLogic *, int, int, std::string);
};
