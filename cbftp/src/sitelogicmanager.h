#pragma once

#include <string>
#include <vector>

#include "core/pointer.h"

class SiteLogic;

class SiteLogicManager {
  private:
    std::vector<Pointer<SiteLogic> > sitelogics;
  public:
    SiteLogicManager();
    const Pointer<SiteLogic> getSiteLogic(const std::string &);
    const Pointer<SiteLogic> getSiteLogic(SiteLogic *);
    void deleteSiteLogic(const std::string &);
};
