#pragma once

#include <string>
#include <vector>
#include <map>
#include <list>

class Site;

#define DEFAULTUSERNAME "anonymous"
#define DEFAULTPASSWORD "anonymous"
#define DEFAULTMAXLOGINS 3
#define DEFAULTMAXUP 0
#define DEFAULTMAXDOWN 2
#define DEFAULTMAXIDLETIME 60
#define DEFAULTSSL true
#define DEFAULTSSLTRANSFER SITE_SSL_PREFER_OFF
#define DEFAULTGLOBALRANK (SITE_RANK_MAX / 2)
#define DEFAULTGLOBALRANKTOLERANCE DEFAULTGLOBALRANK  // Allow all pairings by default

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
    int globalrank;
    int globalranktolerance;
  public:
    SiteManager();
    void readConfiguration();
    int getNumSites() const;
    void addSite(Site *);
    Site * getSite(std::string) const;
    void deleteSite(std::string);
    std::vector<Site *>::const_iterator begin() const;
    std::vector<Site *>::const_iterator end() const;
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
    int getGlobalRank() const;
    void setGlobalRank(int);
    int getGlobalRankTolerance() const;
    void setGlobalRankTolerance(int);
    void writeState();
    void sortSites();
    void proxyRemoved(std::string);
    void addBlockedPair(std::string, std::string);
    bool isBlockedPair(Site *, Site *) const;
    void clearBlocksForSite(Site *);
    std::list<Site *> getBlocksFromSite(Site *) const;
    std::list<Site *> getBlocksToSite(Site *) const;
    bool testRankCompatibility(const Site&, const Site&) const;
};

bool siteNameComparator(Site *, Site *);
