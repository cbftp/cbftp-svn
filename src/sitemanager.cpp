#include "sitemanager.h"

SiteManager::SiteManager() {

}

void SiteManager::readSites() {
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
      addSite(site);
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
    else if (!setting.compare("pret")) {
      if (!value.compare("true")) site->setPRET(true);
    }
    else if (!setting.compare("sslfxpforced")) {
      if (!value.compare("true")) site->setSSLFXPForced(true);
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
  }
}

void SiteManager::writeState() {
  std::vector<Site *>::iterator it;
  DataFileHandler * filehandler = global->getDataFileHandler();
  for (it = sites.begin(); it != sites.end(); it++) {
    Site * site = *it;
    std::string name = site->getName();
    filehandler->addOutputLine("SiteManager", name + "$addr=" + site->getAddress());
    filehandler->addOutputLine("SiteManager", name + "$port=" + site->getPort());
    filehandler->addOutputLine("SiteManager", name + "$user=" + site->getUser());
    filehandler->addOutputLine("SiteManager", name + "$pass=" + site->getPass());
    filehandler->addOutputLine("SiteManager", name + "$logins=" + global->int2Str(site->getInternMaxLogins()));
    filehandler->addOutputLine("SiteManager", name + "$maxup=" + global->int2Str(site->getInternMaxUp()));
    filehandler->addOutputLine("SiteManager", name + "$maxdn=" + global->int2Str(site->getInternMaxDown()));
    if (site->needsPRET()) filehandler->addOutputLine("SiteManager", name + "$pret=true");
    if (site->SSLFXPForced()) filehandler->addOutputLine("SiteManager", name + "$sslfxpforced=true");
    if (site->hasBrokenPASV()) filehandler->addOutputLine("SiteManager", name + "$brokenpasv=true");
    std::map<std::string, std::string>::iterator sit;
    for (sit = site->sectionsBegin(); sit != site->sectionsEnd(); sit++) {
      filehandler->addOutputLine("SiteManager", name + "$section=" + sit->first + "$" + sit->second);
    }
    std::map<std::string, int>::iterator sit2;
    for (sit2 = site->avgspeedBegin(); sit2 != site->avgspeedEnd(); sit2++) {
      filehandler->addOutputLine("SiteManager", name + "$avgspeed=" + sit2->first + "$" + global->int2Str(sit2->second));
    }
  }
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
      return;
    }
  }
}

void SiteManager::addSite(Site * site) {
  sites.push_back(site);
}

std::vector<Site *>::iterator SiteManager::getSitesIteratorBegin() {
  return sites.begin();
}

std::vector<Site *>::iterator SiteManager::getSitesIteratorEnd() {
  return sites.end();
}
