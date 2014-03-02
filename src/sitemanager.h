#pragma once

#include <string>
#include <vector>

#include "site.h"

#define DEFAULTUSERNAME "anonymous"
#define DEFAULTPASSWORD "anonymous"
#define DEFAULTMAXLOGINS 3
#define DEFAULTMAXUP 0
#define DEFAULTMAXDOWN 2
#define DEFAULTMAXIDLETIME 60
#define DEFAULTSSL true
#define DEFAULTSSLTRANSFER SITE_SSL_PREFER_OFF

class SiteManager {
  private:
    std::vector<Site *> sites;
    std::map<Site *, std::map<Site *, bool> > blockedpairs;
    std::string defaultusername;
    std::string defaultpassword;
    unsigned int defaultmaxlogins;
    unsigned int defaultmaxup;
    unsigned int defaultmaxdown;
    unsigned int defaultmaxidletime;
    int defaultssltransfer;
    bool defaultsslconn;
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
    unsigned int getDefaultMaxIdleTime();
    void setDefaultMaxIdleTime(unsigned int);
    bool getDefaultSSL();
    void setDefaultSSL(bool);
    int getDefaultSSLTransferPolicy();
    void setDefaultSSLTransferPolicy(int);
    void writeState();
    void sortSites();
    void proxyRemoved(std::string);
    void addBlockedPair(std::string, std::string);
    bool isBlockedPair(Site *, Site *);
    void clearBlocksForSite(Site *);
    std::list<Site *> getBlocksForSite(Site *);
};

bool siteNameComparator(Site *, Site *);
