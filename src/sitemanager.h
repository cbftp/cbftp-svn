#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

class Site;

class SiteManager {
  private:
    void removeSitePairsForSite(const std::shared_ptr<Site> &);
    std::vector<std::shared_ptr<Site> > sites;
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
    void addSite(const std::shared_ptr<Site> &);
    void addSiteLoad(const std::shared_ptr<Site> &);
    std::shared_ptr<Site> getSite(const std::string &) const;
    std::shared_ptr<Site> getSite(unsigned int) const;
    void deleteSite(const std::string &);
    std::vector<std::shared_ptr<Site> >::const_iterator begin() const;
    std::vector<std::shared_ptr<Site> >::const_iterator end() const;
    std::string getDefaultUserName() const;
    void setDefaultUserName(const std::string &);
    std::string getDefaultPassword() const;
    void setDefaultPassword(const std::string &);
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
    void proxyRemoved(const std::string &);
    void resetSitePairsForSite(const std::string &);
    void addExceptSourceForSite(const std::string &, const std::string &);
    void addExceptTargetForSite(const std::string &, const std::string &);
};
