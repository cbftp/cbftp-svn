#pragma once

#include <sstream>
#include <fstream>
#include <vector>
#include <stdlib.h>
#include <iostream>
#include <algorithm>

#include "globalcontext.h"
#include "site.h"
#include "datafilehandler.h"

#define DEFAULTUSERNAME "anonymous"
#define DEFAULTPASSWORD "anonymous"
#define DEFAULTMAXLOGINS 3
#define DEFAULTMAXUP 0
#define DEFAULTMAXDOWN 2
#define DEFAULTSSLFXPFORCED false

extern GlobalContext * global;

class SiteManager {
  private:
    std::vector<Site *> sites;
    std::string defaultusername;
    std::string defaultpassword;
    unsigned int defaultmaxlogins;
    unsigned int defaultmaxup;
    unsigned int defaultmaxdown;
    bool defaultsslfxpforced;
  public:
    SiteManager();
    void readConfiguration();
    int getNumSites();
    void addSite(Site *);
    Site * getSite(std::string);
    void deleteSite(std::string);
    std::vector<Site *>::iterator getSitesIteratorBegin();
    std::vector<Site *>::iterator getSitesIteratorEnd();
    std::string getDefaultUserName();
    void setDefaultUserName(std::string);
    std::string getDefaultPassword();
    void setDefaultPassword(std::string);
    unsigned int getDefaultMaxLogins();
    void setDefaultMaxLogins(unsigned int);
    unsigned int getDefaultMaxUp();
    void setDefaultMaxUp(unsigned int);
    unsigned int getDefaultMaxDown();
    void setDefaultMaxDown(unsigned int);
    bool getDefaultSSLFXPForced();
    void setDefaultSSLFXPForced(bool);
    void writeState();
};

bool siteNameComparator(Site *, Site *);
