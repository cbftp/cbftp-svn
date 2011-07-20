#pragma once
#include <sstream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <iostream>

#include "globalcontext.h"
#include "site.h"

#define SITEDB "sitedb"

extern GlobalContext * global;

class SiteManager {
  private:
    std::vector<Site *> sites;
  public:
    SiteManager();
    int getNumSites();
    void addSite(Site *);
    Site * getSite(std::string);
    void deleteSite(std::string);
    std::vector<Site *>::iterator getSitesIteratorBegin();
    std::vector<Site *>::iterator getSitesIteratorEnd();
    void writeDataFile();
};
