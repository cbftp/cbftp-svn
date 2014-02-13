#include "sitemanager.h"

#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>

#include "globalcontext.h"
#include "datafilehandler.h"
#include "connstatetracker.h"
#include "eventlog.h"

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
      site->setMaxIdleTime(global->str2Int(value));
    }
    else if (!setting.compare("pret")) {
      if (!value.compare("true")) site->setPRET(true);
    }
    else if (!setting.compare("sslconn")) {
      if (!value.compare("false")) site->setSSL(false);
    }
    else if (!setting.compare("sslfxpforced")) {
      if (!value.compare("true")) site->setSSLTransferPolicy(SITE_SSL_ALWAYS_ON);
    }
    else if (!setting.compare("ssltransfer")) {
      site->setSSLTransferPolicy(global->str2Int(value));
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
      site->setMaxLogins(global->str2Int(value));
    }
    else if (!setting.compare("maxdn")) {
      site->setMaxDn(global->str2Int(value));
    }
    else if (!setting.compare("maxup")) {
      site->setMaxUp(global->str2Int(value));
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
      int avgspeed = global->str2Int(value.substr(split + 1));
      site->setAverageSpeed(sitename, avgspeed);
    }
    else if (!setting.compare("affil")) {
      site->addAffil(value);
    }
    else if (!setting.compare("proxytype")) {
      site->setProxyType(global->str2Int(value));
    }
    else if (!setting.compare("proxyname")) {
      site->setProxy(value);
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
      setDefaultMaxLogins(global->str2Int(value));
    }
    else if (!setting.compare("maxup")) {
      setDefaultMaxUp(global->str2Int(value));
    }
    else if (!setting.compare("maxdown")) {
      setDefaultMaxDown(global->str2Int(value));
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
      setDefaultSSLTransferPolicy(global->str2Int(value));
    }
    else if (!setting.compare("maxidletime")) {
      setDefaultMaxIdleTime(global->str2Int(value));
    }
  }
  std::sort(sites.begin(), sites.end(), siteNameComparator);
  global->getEventLog()->log("SiteManager", "Loaded " + global->int2Str((int)sites.size()) + " sites.");
}

void SiteManager::writeState() {
  global->getEventLog()->log("SiteManager", "Writing state...");
  std::vector<Site *>::iterator it;
  std::string filetag = "SiteManager";
  std::string defaultstag = "SiteManagerDefaults";
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
    filehandler->addOutputLine(filetag, name + "$logins=" + global->int2Str(site->getInternMaxLogins()));
    filehandler->addOutputLine(filetag, name + "$maxup=" + global->int2Str(site->getInternMaxUp()));
    filehandler->addOutputLine(filetag, name + "$maxdn=" + global->int2Str(site->getInternMaxDown()));
    filehandler->addOutputLine(filetag, name + "$idletime=" + global->int2Str(site->getMaxIdleTime()));
    filehandler->addOutputLine(filetag, name + "$ssltransfer=" + global->int2Str(site->getSSLTransferPolicy()));
    if (site->needsPRET()) filehandler->addOutputLine(filetag, name + "$pret=true");
    if (!site->SSL()) filehandler->addOutputLine(filetag, name + "$sslconn=false");
    if (!site->getAllowUpload()) filehandler->addOutputLine(filetag, name + "$allowupload=false");
    if (!site->getAllowDownload()) filehandler->addOutputLine(filetag, name + "$allowdownload=false");
    if (site->hasBrokenPASV()) filehandler->addOutputLine(filetag, name + "$brokenpasv=true");
    int proxytype = site->getProxyType();
    filehandler->addOutputLine(filetag, name + "$proxytype=" + global->int2Str(proxytype));
    if (proxytype == SITE_PROXY_USE) {
      filehandler->addOutputLine(filetag, name + "$proxyname=" + site->getProxy());
    }
    std::map<std::string, std::string>::iterator sit;
    for (sit = site->sectionsBegin(); sit != site->sectionsEnd(); sit++) {
      filehandler->addOutputLine(filetag, name + "$section=" + sit->first + "$" + sit->second);
    }
    std::map<std::string, int>::iterator sit2;
    for (sit2 = site->avgspeedBegin(); sit2 != site->avgspeedEnd(); sit2++) {
      filehandler->addOutputLine(filetag, name + "$avgspeed=" + sit2->first + "$" + global->int2Str(sit2->second));
    }
    std::map<std::string, bool>::iterator sit3;
    for (sit3 = site->affilsBegin(); sit3 != site->affilsEnd(); sit3++) {
      filehandler->addOutputLine(filetag, name + "$affil=" + sit3->first);
    }
  }
  filehandler->addOutputLine(defaultstag, "username=" + getDefaultUserName());
  filehandler->addOutputLine(defaultstag, "password=" + getDefaultPassword());
  filehandler->addOutputLine(defaultstag, "maxlogins=" + global->int2Str(getDefaultMaxLogins()));
  filehandler->addOutputLine(defaultstag, "maxup=" + global->int2Str(getDefaultMaxUp()));
  filehandler->addOutputLine(defaultstag, "maxdown=" + global->int2Str(getDefaultMaxDown()));
  filehandler->addOutputLine(defaultstag, "maxidletime=" + global->int2Str(getDefaultMaxIdleTime()));
  filehandler->addOutputLine(defaultstag, "ssltransfer=" + global->int2Str(getDefaultSSLTransferPolicy()));
  if (!getDefaultSSL()) filehandler->addOutputLine(defaultstag, "sslconn=false");
}

int SiteManager::getNumSites() {
  return sites.size();
}

Site * SiteManager::getSite(std::string site) {
  std::vector<Site *>::iterator it;
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

std::vector<Site *>::iterator SiteManager::getSitesIteratorBegin() {
  return sites.begin();
}

std::vector<Site *>::iterator SiteManager::getSitesIteratorEnd() {
  return sites.end();
}

bool siteNameComparator(Site * a, Site * b) {
  return a->getName().compare(b->getName()) < 0;
}

std::string SiteManager::getDefaultUserName() {
  return defaultusername;
}

void SiteManager::setDefaultUserName(std::string username) {
  defaultusername = username;
}

std::string SiteManager::getDefaultPassword() {
  return defaultpassword;
}

void SiteManager::setDefaultPassword(std::string password) {
  defaultpassword = password;
}

unsigned int SiteManager::getDefaultMaxLogins() {
  return defaultmaxlogins;
}

void SiteManager::setDefaultMaxLogins(unsigned int maxlogins) {
  defaultmaxlogins = maxlogins;
}

unsigned int SiteManager::getDefaultMaxUp() {
  return defaultmaxup;
}

void SiteManager::setDefaultMaxUp(unsigned int maxup) {
  defaultmaxup = maxup;
}

unsigned int SiteManager::getDefaultMaxDown() {
  return defaultmaxdown;
}

void SiteManager::setDefaultMaxDown(unsigned int maxdown) {
  defaultmaxdown = maxdown;
}

unsigned int SiteManager::getDefaultMaxIdleTime() {
  return defaultmaxidletime;
}

void SiteManager::setDefaultMaxIdleTime(unsigned int idletime) {
  defaultmaxidletime = idletime;
}

bool SiteManager::getDefaultSSL() {
  return defaultsslconn;
}

void SiteManager::setDefaultSSL(bool ssl) {
  defaultsslconn = ssl;
}

int SiteManager::getDefaultSSLTransferPolicy() {
  return defaultssltransfer;
}

void SiteManager::setDefaultSSLTransferPolicy(int policy) {
  defaultssltransfer = policy;
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
