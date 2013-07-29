#pragma once

#include <string>

#include "sitelogic.h"

class SiteLogicManager {
  private:
    std::vector<SiteLogic *> sitelogics;
  public:
    SiteLogicManager();
    SiteLogic * getSiteLogic(std::string);
    void deleteSiteLogic(std::string);
};
