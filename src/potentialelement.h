#pragma once

#include <string>

#include "core/pointer.h"

class SiteLogic;

class PotentialElement {
  private:
    Pointer<SiteLogic> site;
    int potential;
    int dstupslots;
    std::string filename;
  public:
    PotentialElement();
    const Pointer<SiteLogic> & getSite() const;
    int getDestinationSiteUploadSlots() const;
    int getPotential() const;
    std::string getFileName() const;
    void reset();
    void update(const Pointer<SiteLogic> &, int, int, const std::string &);
};
