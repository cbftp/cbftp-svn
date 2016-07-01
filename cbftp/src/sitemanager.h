#pragma once

#include <string>
#include <vector>
#include <map>
#include <list>

class Site;

class SiteManager {
  private:
    void removeSitePairsForSite(Site *);
    std::vector<Site *> sites;
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
    int getNumSites() const;
    void addSite(Site *);
    void addSiteLoad(Site *);
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
    void sortSites();
    void proxyRemoved(std::string);
    void resetSitePairsForSite(const std::string &);
    void addExceptSourceForSite(const std::string &, const std::string &);
    void addExceptTargetForSite(const std::string &, const std::string &);
};
