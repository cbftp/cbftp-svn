#pragma once
#include <string>
#include "sitethread.h"

class SiteThreadManager {
  private:
    std::map<std::string, SiteThread *> sitethreads;
  public:
    SiteThreadManager();
    SiteThread * getSiteThread(std::string);
};
