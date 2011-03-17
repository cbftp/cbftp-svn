#pragma once
#include <string>
#include "sitethread.h"

class SiteThreadManager {
  private:
    std::vector<SiteThread *> sitethreads;
  public:
    SiteThreadManager();
    SiteThread * getSiteThread(std::string);
};
