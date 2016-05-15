#pragma once

#include <string>

class SiteLogic;

class PotentialElement {
  private:
    SiteLogic * site;
    int potential;
    int dnslots;
    std::string filename;
  public:
    PotentialElement();
    SiteLogic * getSite() const;
    int getSiteDownloadSlots() const;
    int getPotential() const;
    std::string getFileName() const;
    void reset();
    void update(SiteLogic *, int, int, const std::string &);
};
