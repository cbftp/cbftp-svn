#include "sitemanager.h"

SiteManager::SiteManager() {
  std::fstream sitedbfile;
  sitedbfile.open(SITEDB);
  std::string line;
  while (std::getline(sitedbfile, line)) {
    if (line.length() == 0 ||line[0] == '#') continue;
    int tok1 = line.find('$');
    int tok2 = line.find('=', tok1);
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
      int split = value.find('$');
      std::string sectionname = value.substr(0, split);
      std::string sectionpath = value.substr(split + 1);
      site->addSection(sectionname, sectionpath);
    }
    else if (!setting.compare("avgspeed")) {
      int split = value.find('$');
      std::string sitename = value.substr(0, split);
      int avgspeed = global->str2Int(value.substr(split + 1));
      site->setAverageSpeed(sitename, avgspeed);
    }
  }
  sitedbfile.close();
}

void SiteManager::writeDataFile() {
  std::ofstream sitedbfile;
  sitedbfile.open (SITEDB, std::ios::trunc);
  sitedbfile << "# this is an example entry\n#SITENAME$addr=123.345.567.789\n#SITENAME$port=1337\n#SITENAME$user=yourusername\n#SITENAME$pass=yourpassword\n#SITENAME$logins=3\n#SITENAME$maxup=2\n#SITENAME$maxdn=1\n#SITENAME$section=TV-X264$/tv-720p\n#SITENAME$section=GAMES$/games\n" << std::endl;
  std::vector<Site *>::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    Site * site = *it;
    std::string name = site->getName();
    sitedbfile << name << "$addr=" << site->getAddress() << std::endl;
    sitedbfile << name << "$port=" << site->getPort() << std::endl;
    sitedbfile << name << "$user=" << site->getUser() << std::endl;
    sitedbfile << name << "$pass=" << site->getPass() << std::endl;
    sitedbfile << name << "$logins=" << site->getMaxLogins() << std::endl;
    sitedbfile << name << "$maxup=" << site->getMaxUp() << std::endl;
    sitedbfile << name << "$maxdn=" << site->getMaxDown() << std::endl;
    if (site->needsPRET()) sitedbfile << name << "$pret=true" << std::endl;
    if (site->hasBrokenPASV()) sitedbfile << name << "$brokenpasv=true" << std::endl;
    std::map<std::string, std::string>::iterator sit;
    for (sit = site->sectionsBegin(); sit != site->sectionsEnd(); sit++) {
      sitedbfile << name << "$section=" << sit->first << "$" << sit->second << std::endl;
    }
    std::map<std::string, int>::iterator sit2;
    for (sit2 = site->avgspeedBegin(); sit2 != site->avgspeedEnd(); sit2++) {
      sitedbfile << name << "$avgspeed=" << sit2->first << "$" << sit2->second << std::endl;
    }
  }
  sitedbfile.close();
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
