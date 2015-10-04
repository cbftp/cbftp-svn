#include "sitemanager.h"

#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>

#include "globalcontext.h"
#include "datafilehandler.h"
#include "connstatetracker.h"
#include "eventlog.h"
#include "util.h"
#include "site.h"

extern GlobalContext * global;

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

void SiteManager::readConfiguration() {
  std::vector<std::string> lines;
  global->getDataFileHandler()->getDataFor("SiteManager", &lines);
  std::vector<std::string>::iterator it;
  std::string line;
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok1 = line.find('$');
    size_t tok2 = line.find('=', tok1);
    std::string name = line.substr(0, tok1);
    std::string setting = line.substr(tok1 + 1, (tok2 - tok1 - 1));
    std::string value = line.substr(tok2 + 1);
    Site * site = getSite(name);
    if (site == NULL) {
      site = new Site(name);
      sites.push_back(site);
    }
    if (!setting.compare("addr")) {
      site->setAddress(value);
    }
    else if (!setting.compare("port")) {
      site->setPort(value);
    }
    else if (!setting.compare("user")) {
      site->setUser(value);
    }
    else if (!setting.compare("pass")) {
      site->setPass(value);
    }
    else if (!setting.compare("basepath")) {
      site->setBasePath(value);
    }
    else if (!setting.compare("idletime")) {
      site->setMaxIdleTime(util::str2Int(value));
    }
    else if (!setting.compare("pret")) {
      if (!value.compare("true")) site->setPRET(true);
    }
    else if (!setting.compare("binary")) {
      if (!value.compare("true")) site->setForceBinaryMode(true);
    }
    else if (!setting.compare("sslconn")) {
      if (!value.compare("false")) site->setSSL(false);
    }
    else if (!setting.compare("sslfxpforced")) {
      if (!value.compare("true")) site->setSSLTransferPolicy(SITE_SSL_ALWAYS_ON);
    }
    else if (!setting.compare("ssltransfer")) {
      site->setSSLTransferPolicy(util::str2Int(value));
    }
    else if (!setting.compare("cpsv")) {
      if (!value.compare("false")) site->setSupportsCPSV(false);
    }
    else if (!setting.compare("listcommand")) {
      site->setListCommand(util::str2Int(value));
    }
    else if (!setting.compare("allowupload")) {
      if (!value.compare("false")) site->setAllowUpload(false);
    }
    else if (!setting.compare("allowdownload")) {
      if (!value.compare("false")) site->setAllowDownload(false);
    }
    else if (!setting.compare("brokenpasv")) {
      if (!value.compare("true")) site->setBrokenPASV(true);
    }
    else if (!setting.compare("logins")) {
      site->setMaxLogins(util::str2Int(value));
    }
    else if (!setting.compare("maxdn")) {
      site->setMaxDn(util::str2Int(value));
    }
    else if (!setting.compare("maxup")) {
      site->setMaxUp(util::str2Int(value));
    }
    else if (!setting.compare("section")) {
      size_t split = value.find('$');
      std::string sectionname = value.substr(0, split);
      std::string sectionpath = value.substr(split + 1);
      site->addSection(sectionname, sectionpath);
    }
    else if (!setting.compare("avgspeed")) {
      size_t split = value.find('$');
      std::string sitename = value.substr(0, split);
      int avgspeed = util::str2Int(value.substr(split + 1));
      site->setAverageSpeed(sitename, avgspeed);
    }
    else if (!setting.compare("affil")) {
      site->addAffil(value);
    }
    else if (!setting.compare("bannedgroup")) {
      site->addBannedGroup(value);
    }
    else if (!setting.compare("proxytype")) {
      site->setProxyType(util::str2Int(value));
    }
    else if (!setting.compare("proxyname")) {
      site->setProxy(value);
    }
    else if (!setting.compare("rank")) {
      site->setRank(util::str2Int(value));
    }
    else if (!setting.compare("ranktolerance")) {
      site->setRankTolerance(util::str2Int(value));
    }
  }
  lines.clear();
  global->getDataFileHandler()->getDataFor("SiteManagerDefaults", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("username")) {
      setDefaultUserName(value);
    }
    else if (!setting.compare("password")) {
      setDefaultPassword(value);
    }
    else if (!setting.compare("maxlogins")) {
      setDefaultMaxLogins(util::str2Int(value));
    }
    else if (!setting.compare("maxup")) {
      setDefaultMaxUp(util::str2Int(value));
    }
    else if (!setting.compare("maxdown")) {
      setDefaultMaxDown(util::str2Int(value));
    }
    else if (!setting.compare("sslconn")) {
      if (!value.compare("false")) {
        setDefaultSSL(false);
      }
    }
    else if (!setting.compare("sslfxpforced")) {
      if (!value.compare("true")) {
        setDefaultSSLTransferPolicy(SITE_SSL_ALWAYS_ON);
      }
    }
    else if (!setting.compare("ssltransfer")) {
      setDefaultSSLTransferPolicy(util::str2Int(value));
    }
    else if (!setting.compare("maxidletime")) {
      setDefaultMaxIdleTime(util::str2Int(value));
    }
    else if (!setting.compare("rank")) {
      setGlobalRank(util::str2Int(value));
    }
    else if (!setting.compare("ranktolerance")) {
      setGlobalRankTolerance(util::str2Int(value));
    }
  }
  lines.clear();
  global->getDataFileHandler()->getDataFor("SiteManagerRules", &lines);
  for (it = lines.begin(); it != lines.end(); it++) {
    line = *it;
    if (line.length() == 0 ||line[0] == '#') continue;
    size_t tok = line.find('=');
    std::string setting = line.substr(0, tok);
    std::string value = line.substr(tok + 1);
    if (!setting.compare("blockedpair")) {
      size_t split = value.find('$');
      std::string site1 = value.substr(0, split);
      std::string site2 = value.substr(split + 1);
      addBlockedPair(site1, site2);
    }
  }
  std::sort(sites.begin(), sites.end(), siteNameComparator);
  global->getEventLog()->log("SiteManager", "Loaded " + util::int2Str((int)sites.size()) + " sites.");
}

void SiteManager::writeState() {
  global->getEventLog()->log("SiteManager", "Writing state...");
  std::vector<Site *>::iterator it;
  std::string filetag = "SiteManager";
  std::string defaultstag = "SiteManagerDefaults";
  std::string rulestag = "SiteManagerRules";
  DataFileHandler * filehandler = global->getDataFileHandler();
  for (it = sites.begin(); it != sites.end(); it++) {
    Site * site = *it;
    std::string name = site->getName();
    filehandler->addOutputLine(filetag, name + "$addr=" + site->getAddress());
    filehandler->addOutputLine(filetag, name + "$port=" + site->getPort());
    filehandler->addOutputLine(filetag, name + "$user=" + site->getUser());
    filehandler->addOutputLine(filetag, name + "$pass=" + site->getPass());
    std::string basepath = site->getBasePath();
    if (basepath != "" && basepath != "/") {
      filehandler->addOutputLine(filetag, name + "$basepath=" + basepath);
    }
    filehandler->addOutputLine(filetag, name + "$logins=" + util::int2Str(site->getInternMaxLogins()));
    filehandler->addOutputLine(filetag, name + "$maxup=" + util::int2Str(site->getInternMaxUp()));
    filehandler->addOutputLine(filetag, name + "$maxdn=" + util::int2Str(site->getInternMaxDown()));
    filehandler->addOutputLine(filetag, name + "$idletime=" + util::int2Str(site->getMaxIdleTime()));
    filehandler->addOutputLine(filetag, name + "$ssltransfer=" + util::int2Str(site->getSSLTransferPolicy()));
    if (!site->supportsCPSV()) filehandler->addOutputLine(filetag, name + "$cpsv=false");
    filehandler->addOutputLine(filetag, name + "$listcommand=" + util::int2Str(site->getListCommand()));
    if (site->needsPRET()) filehandler->addOutputLine(filetag, name + "$pret=true");
    if (site->forceBinaryMode()) filehandler->addOutputLine(filetag, name + "$binary=true");
    if (!site->SSL()) filehandler->addOutputLine(filetag, name + "$sslconn=false");
    if (!site->getAllowUpload()) filehandler->addOutputLine(filetag, name + "$allowupload=false");
    if (!site->getAllowDownload()) filehandler->addOutputLine(filetag, name + "$allowdownload=false");
    if (site->hasBrokenPASV()) filehandler->addOutputLine(filetag, name + "$brokenpasv=true");
    filehandler->addOutputLine(filetag, name + "$rank=" + util::int2Str(site->getRank()));
    filehandler->addOutputLine(filetag, name + "$ranktolerance=" + util::int2Str(site->getRankTolerance()));
    int proxytype = site->getProxyType();
    filehandler->addOutputLine(filetag, name + "$proxytype=" + util::int2Str(proxytype));
    if (proxytype == SITE_PROXY_USE) {
      filehandler->addOutputLine(filetag, name + "$proxyname=" + site->getProxy());
    }
    std::map<std::string, std::string>::const_iterator sit;
    for (sit = site->sectionsBegin(); sit != site->sectionsEnd(); sit++) {
      filehandler->addOutputLine(filetag, name + "$section=" + sit->first + "$" + sit->second);
    }
    std::map<std::string, int>::const_iterator sit2;
    for (sit2 = site->avgspeedBegin(); sit2 != site->avgspeedEnd(); sit2++) {
      filehandler->addOutputLine(filetag, name + "$avgspeed=" + sit2->first + "$" + util::int2Str(sit2->second));
    }
    std::map<std::string, bool>::const_iterator sit3;
    for (sit3 = site->affilsBegin(); sit3 != site->affilsEnd(); sit3++) {
      filehandler->addOutputLine(filetag, name + "$affil=" + sit3->first);
    }
    for (sit3 = site->bannedGroupsBegin(); sit3 != site->bannedGroupsEnd(); sit3++) {
      filehandler->addOutputLine(filetag, name + "$bannedgroup=" + sit3->first);
    }
  }
  filehandler->addOutputLine(defaultstag, "username=" + getDefaultUserName());
  filehandler->addOutputLine(defaultstag, "password=" + getDefaultPassword());
  filehandler->addOutputLine(defaultstag, "maxlogins=" + util::int2Str(getDefaultMaxLogins()));
  filehandler->addOutputLine(defaultstag, "maxup=" + util::int2Str(getDefaultMaxUp()));
  filehandler->addOutputLine(defaultstag, "maxdown=" + util::int2Str(getDefaultMaxDown()));
  filehandler->addOutputLine(defaultstag, "maxidletime=" + util::int2Str(getDefaultMaxIdleTime()));
  filehandler->addOutputLine(defaultstag, "ssltransfer=" + util::int2Str(getDefaultSSLTransferPolicy()));
  filehandler->addOutputLine(defaultstag, "rank=" + util::int2Str(getGlobalRank()));
  filehandler->addOutputLine(defaultstag, "ranktolerance=" + util::int2Str(getGlobalRankTolerance()));
  if (!getDefaultSSL()) filehandler->addOutputLine(defaultstag, "sslconn=false");
  std::map<Site *, std::map<Site *, bool> >::iterator it2;
  std::map<Site *, bool>::iterator it3;
  for (it2 = blockedpairs.begin(); it2 != blockedpairs.end(); it2++) {
    for (it3 = it2->second.begin(); it3 != it2->second.end(); it3++) {
      filehandler->addOutputLine(rulestag, "blockedpair=" + it2->first->getName() + "$" + it3->first->getName());
    }
  }
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

