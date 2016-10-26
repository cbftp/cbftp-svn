#include "sitemanager.h"

#include <sstream>
#include <fstream>
#include <cstdlib>
#include <algorithm>
#include <set>

#include "globalcontext.h"
#include "connstatetracker.h"
#include "eventlog.h"
#include "util.h"
#include "site.h"

#define DEFAULTUSERNAME "anonymous"
#define DEFAULTPASSWORD "anonymous"
#define DEFAULTMAXLOGINS 3
#define DEFAULTMAXUP 0
#define DEFAULTMAXDOWN 2
#define DEFAULTMAXIDLETIME 60
#define DEFAULTSSL true
#define DEFAULTSSLTRANSFER SITE_SSL_PREFER_OFF

bool siteNameComparator(const Pointer<Site> & a, const Pointer<Site> & b) {
  return a->getName().compare(b->getName()) < 0;
}

SiteManager::SiteManager() :
  defaultusername(DEFAULTUSERNAME),
  defaultpassword(DEFAULTPASSWORD),
  defaultmaxlogins(DEFAULTMAXLOGINS),
  defaultmaxup(DEFAULTMAXUP),
  defaultmaxdown(DEFAULTMAXDOWN),
  defaultmaxidletime(DEFAULTMAXIDLETIME),
  defaultssltransfer(DEFAULTSSLTRANSFER),
  defaultsslconn(DEFAULTSSL)
  {
}

int SiteManager::getNumSites() const {
  return sites.size();
}

Pointer<Site> SiteManager::getSite(const std::string & site) const {
  std::vector<Pointer<Site> >::const_iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if ((*it)->getName().compare(site) == 0) {
      return *it;
    }
  }
  return Pointer<Site>();
}

void SiteManager::deleteSite(const std::string & site) {
  std::vector<Pointer<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if ((*it)->getName().compare(site) == 0) {
      removeSitePairsForSite(*it);
      sites.erase(it);
      global->getEventLog()->log("SiteManager", "Site " + site + " deleted.");
      return;
    }
  }
}

void SiteManager::addSite(const Pointer<Site> & site) {
  sites.push_back(site);
  std::set<Pointer<Site> >::const_iterator it;
  for (it = site->exceptSourceSitesBegin(); it != site->exceptSourceSitesEnd(); it++) {
    addExceptSourceForSite(site->getName(), (*it)->getName());
  }
  for (it = site->exceptTargetSitesBegin(); it != site->exceptTargetSitesEnd(); it++) {
    addExceptTargetForSite(site->getName(), (*it)->getName());
  }
  global->getEventLog()->log("SiteManager", "Site " + site->getName() + " added.");
  sortSites();
}

void SiteManager::addSiteLoad(const Pointer<Site> & site) {
  sites.push_back(site);
}

void SiteManager::sortSites() {
  std::sort(sites.begin(), sites.end(), siteNameComparator);
}

std::vector<Pointer<Site> >::const_iterator SiteManager::begin() const {
  return sites.begin();
}

std::vector<Pointer<Site> >::const_iterator SiteManager::end() const {
  return sites.end();
}

std::string SiteManager::getDefaultUserName() const {
  return defaultusername;
}

void SiteManager::setDefaultUserName(const std::string & username) {
  defaultusername = username;
}

std::string SiteManager::getDefaultPassword() const {
  return defaultpassword;
}

void SiteManager::setDefaultPassword(const std::string & password) {
  defaultpassword = password;
}

unsigned int SiteManager::getDefaultMaxLogins() const {
  return defaultmaxlogins;
}

void SiteManager::setDefaultMaxLogins(unsigned int maxlogins) {
  defaultmaxlogins = maxlogins;
}

unsigned int SiteManager::getDefaultMaxUp() const {
  return defaultmaxup;
}

void SiteManager::setDefaultMaxUp(unsigned int maxup) {
  defaultmaxup = maxup;
}

unsigned int SiteManager::getDefaultMaxDown() const {
  return defaultmaxdown;
}

void SiteManager::setDefaultMaxDown(unsigned int maxdown) {
  defaultmaxdown = maxdown;
}

unsigned int SiteManager::getDefaultMaxIdleTime() const {
  return defaultmaxidletime;
}

void SiteManager::setDefaultMaxIdleTime(unsigned int idletime) {
  defaultmaxidletime = idletime;
}

bool SiteManager::getDefaultSSL() const {
  return defaultsslconn;
}

void SiteManager::setDefaultSSL(bool ssl) {
  defaultsslconn = ssl;
}

int SiteManager::getDefaultSSLTransferPolicy() const {
  return defaultssltransfer;
}

void SiteManager::setDefaultSSLTransferPolicy(int policy) {
  defaultssltransfer = policy;
}

void SiteManager::proxyRemoved(const std::string & removedproxy) {
  std::vector<Pointer<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if ((*it)->getProxyType() == SITE_PROXY_USE && (*it)->getProxy() == removedproxy) {
      (*it)->setProxyType(SITE_PROXY_GLOBAL);
      global->getEventLog()->log("SiteManager", "Used proxy (" + removedproxy + ") was removed, reset proxy type for " + (*it)->getName());
    }
  }
}

void SiteManager::removeSitePairsForSite(const Pointer<Site> & site) {
  std::vector<Pointer<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    (*it)->removeExceptSite(site);
  }
}

void SiteManager::resetSitePairsForSite(const std::string & site) {
  Pointer<Site> sitep = getSite(site);
  if (!sitep) {
    return;
  }
  sitep->clearExceptSites();
  std::vector<Pointer<Site> >::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if (*it == sitep) {
      continue;
    }
    if (sitep->getTransferSourcePolicy() == SITE_TRANSFER_POLICY_ALLOW) {
      (*it)->addAllowedTargetSite(sitep);
    }
    else {
      (*it)->addBlockedTargetSite(sitep);
    }
    if (sitep->getTransferTargetPolicy() == SITE_TRANSFER_POLICY_ALLOW) {
      (*it)->addAllowedSourceSite(sitep);
    }
    else {
      (*it)->addBlockedSourceSite(sitep);
    }
  }
}

void SiteManager::addExceptSourceForSite(const std::string & site, const std::string & exceptsite) {
  Pointer<Site> sitep = getSite(site);
  Pointer<Site> exceptsitep = getSite(exceptsite);
  if (!sitep || !exceptsitep || sitep == exceptsitep) {
    return;
  }
  if (sitep->getTransferSourcePolicy() == SITE_TRANSFER_POLICY_ALLOW) {
    sitep->addBlockedSourceSite(exceptsitep);
    exceptsitep->addBlockedTargetSite(sitep);
  }
  else {
    sitep->addAllowedSourceSite(exceptsitep);
    exceptsitep->addAllowedTargetSite(sitep);
  }
}

void SiteManager::addExceptTargetForSite(const std::string & site, const std::string & exceptsite) {
  Pointer<Site> sitep = getSite(site);
  Pointer<Site> exceptsitep = getSite(exceptsite);
  if (!sitep || !exceptsitep || sitep == exceptsitep) {
    return;
  }
  if (sitep->getTransferTargetPolicy() == SITE_TRANSFER_POLICY_ALLOW) {
    sitep->addBlockedTargetSite(exceptsitep);
    exceptsitep->addBlockedSourceSite(sitep);
  }
  else {
    sitep->addAllowedTargetSite(exceptsitep);
    exceptsitep->addAllowedSourceSite(sitep);
  }
}
