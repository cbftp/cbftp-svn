#pragma once
#include <sstream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <iostream>

#include "site.h"

#define SITEDB "sitedb"

class SiteManager {
  private:
    std::map<std::string, Site *> sites;
    int strToInt(std::string);
  public:
    SiteManager();
    Site * getSite(std::string);
    void writeDataFile();
};
