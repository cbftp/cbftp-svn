#pragma once

#include <string>
#include <vector>
#include <map>
#include <list>

#include "core/pointer.h"

class Site;

class SiteManager {
  private:
    void removeSitePairsForSite(const Pointer<Site> &);
    std::vector<Pointer<Site> > sites;
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
    void addSite(const Pointer<Site> &);
    void addSiteLoad(const Pointer<Site> &);
    Pointer<Site> getSite(const std::string &) const;
    void deleteSite(const std::string &);
    std::vector<Pointer<Site> >::const_iterator begin() const;
    std::vector<Pointer<Site> >::const_iterator end() const;
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
