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
    int getNumSites() const;
    void addSite(Site *);
    Site * getSite(std::string) const;
    void deleteSite(std::string);
    std::vector<Site *>::const_iterator getSitesIteratorBegin() const;
    std::vector<Site *>::const_iterator getSitesIteratorEnd() const;
    std::string getDefaultUserName() const;
    void setDefaultUserName(std::string);
    std::string getDefaultPassword() const;
    void setDefaultPassword(std::string);
    unsigned int getDefaultMaxLogins() const;
    void setDefaultMaxLogins(unsigned int);
    unsigned int getDefaultMaxUp() const;
    void setDefaultMaxUp(unsigned int);
    unsigned int getDefaultMaxDown() const;
    void setDefaultMaxDown(unsigned int);
    unsigned int getDefaultMaxIdleTime() const;
    void setDefaultMaxIdleTime(unsigned int);
    bool getDefaultSSL() const;
    void setDefaultSSL(bool);
    int getDefaultSSLTransferPolicy() const;
    void setDefaultSSLTransferPolicy(int);
    void writeState();
    void sortSites();
    void proxyRemoved(std::string);
    void addBlockedPair(std::string, std::string);
    bool isBlockedPair(Site *, Site *) const;
    void clearBlocksForSite(Site *);
    std::list<Site *> getBlocksForSite(Site *);
};

bool siteNameComparator(Site *, Site *);
