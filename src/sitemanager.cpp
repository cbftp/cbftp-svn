#include "sitemanager.h"

#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>

#include "globalcontext.h"
#include "connstatetracker.h"
#include "eventlog.h"
#include "util.h"
#include "site.h"

SiteManager::SiteManager() {
  defaultusername = DEFAULTUSERNAME;
  defaultpassword = DEFAULTPASSWORD;
  defaultmaxlogins = DEFAULTMAXLOGINS;
  defaultmaxup = DEFAULTMAXUP;
  defaultmaxdown = DEFAULTMAXDOWN;
  defaultsslconn = DEFAULTSSL;
  defaultssltransfer = DEFAULTSSLTRANSFER;
  defaultmaxidletime = DEFAULTMAXIDLETIME;
  globalrank = DEFAULTGLOBALRANK;
  globalranktolerance = DEFAULTGLOBALRANKTOLERANCE;
}

int SiteManager::getNumSites() const {
  return sites.size();
}

Site * SiteManager::getSite(std::string site) const {
  std::vector<Site *>::const_iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if ((*it)->getName().compare(site) == 0) {
      return *it;
    }
  }
  return NULL;
}

void SiteManager::deleteSite(std::string site) {
  std::vector<Site *>::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if ((*it)->getName().compare(site) == 0) {
      clearBlocksForSite(*it);
      blockedpairs.erase(*it);
      delete *it;
      sites.erase(it);
      global->getEventLog()->log("SiteManager", "Site " + site + " deleted.");
      return;
    }
  }
}

void SiteManager::addSite(Site * site) {
  sites.push_back(site);
  global->getEventLog()->log("SiteManager", "Site " + site->getName() + " added.");
  sortSites();
}

void SiteManager::addSiteLoad(Site * site) {
  sites.push_back(site);
}

void SiteManager::sortSites() {
  std::sort(sites.begin(), sites.end(), siteNameComparator);
}

std::vector<Site *>::const_iterator SiteManager::begin() const {
  return sites.begin();
}

std::vector<Site *>::const_iterator SiteManager::end() const {
  return sites.end();
}

bool siteNameComparator(Site * a, Site * b) {
  return a->getName().compare(b->getName()) < 0;
}

std::string SiteManager::getDefaultUserName() const {
  return defaultusername;
}

void SiteManager::setDefaultUserName(std::string username) {
  defaultusername = username;
}

std::string SiteManager::getDefaultPassword() const {
  return defaultpassword;
}

void SiteManager::setDefaultPassword(std::string password) {
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

int SiteManager::getGlobalRank() const {
  return globalrank;
}

void SiteManager::setGlobalRank(int rank) {
  globalrank = rank;
}

int SiteManager::getGlobalRankTolerance() const {
  return globalranktolerance;
}

void SiteManager::setGlobalRankTolerance(int tolerance) {
  globalranktolerance = tolerance;
}

void SiteManager::proxyRemoved(std::string removedproxy) {
  std::vector<Site *>::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    if ((*it)->getProxyType() == SITE_PROXY_USE && (*it)->getProxy() == removedproxy) {
      (*it)->setProxyType(SITE_PROXY_GLOBAL);
      global->getEventLog()->log("SiteManager", "Used proxy (" + removedproxy + ") was removed, reset proxy type for " + (*it)->getName());
    }
  }
}

void SiteManager::addBlockedPair(std::string sitestr1, std::string sitestr2) {
  Site * site1 = getSite(sitestr1);
  Site * site2 = getSite(sitestr2);
  if (site1 == NULL || site2 == NULL) {
    return;
  }
  if (blockedpairs.find(site1) == blockedpairs.end()) {
    blockedpairs[site1] = std::map<Site *, bool>();
  }
  blockedpairs[site1][site2] = true;
}

bool SiteManager::isBlockedPair(Site * site1, Site * site2) const {
  std::map<Site *, std::map<Site *, bool> >::const_iterator it = blockedpairs.find(site1);
  if (it == blockedpairs.end()) {
    return false;
  }
  return it->second.find(site2) != it->second.end();
}

void SiteManager::clearBlocksForSite(Site * site) {
  std::map<Site *, std::map<Site *, bool> >::iterator it = blockedpairs.find(site);
  if (it != blockedpairs.end()) {
    it->second = std::map<Site *, bool>();
  }
  for (it = blockedpairs.begin(); it != blockedpairs.end(); it++) {
    if (it->second.find(site) != it->second.end()) {
      it->second.erase(site);
    }
  }
}

std::list<Site *> SiteManager::getBlocksFromSite(Site * site) const {
  std::list<Site *> blockedlist;
  std::map<Site *, std::map<Site *, bool> >::const_iterator it = blockedpairs.find(site);
  if (it == blockedpairs.end()) {
    return blockedlist;
  }
  std::map<Site *, bool>::const_iterator it2;
  for (it2 = it->second.begin(); it2 != it->second.end(); it2++) {
    blockedlist.push_back(it2->first);
  }
  return blockedlist;
}

std::list<Site *> SiteManager::getBlocksToSite(Site * site) const {
  std::list<Site *> blockedlist;
  std::map<Site *, std::map<Site *, bool> >::const_iterator it;
  for (it = blockedpairs.begin(); it != blockedpairs.end(); it++) {
    if (it->second.find(site) != it->second.end()) {
      blockedlist.push_back(it->first);
    }
  }
  return blockedlist;
}

std::map<Site *, std::map<Site *, bool> >::const_iterator SiteManager::blockedPairsBegin() const {
  return blockedpairs.begin();
}

std::map<Site *, std::map<Site *, bool> >::const_iterator SiteManager::blockedPairsEnd() const {
  return blockedpairs.end();
}

bool SiteManager::testRankCompatibility(const Site& src, const Site& dst) const {
  int srcrank = src.getRank();
  int dstrank = dst.getRank();
  int srctolerance = src.getRankTolerance();
  
  if (srcrank == SITE_RANK_USE_GLOBAL)
    srcrank = getGlobalRank();

  if (dstrank == SITE_RANK_USE_GLOBAL)
    dstrank = getGlobalRank();

  if (srctolerance == SITE_RANK_USE_GLOBAL) 
    srctolerance = getGlobalRankTolerance();

  return (dstrank >= (srcrank - srctolerance));
}

