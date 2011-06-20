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
    std::map<std::string, Site *> sites;
  public:
    SiteManager();
    int getNumSites();
    Site * getSite(std::string);
    std::map<std::string, Site *>::iterator getSitesIteratorBegin();
    std::map<std::string, Site *>::iterator getSitesIteratorEnd();
    void writeDataFile();
};
