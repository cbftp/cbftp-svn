#include "sitemanager.h"

SiteManager::SiteManager() {
  std::ifstream sitedbfile;
  sitedbfile.open(SITEDB);
  if (!sitedbfile) {
    std::cerr << "Error: Unable to open sitedb file" << std::endl;
    exit(1);
  }
  std::string line;
  while (std::getline(sitedbfile, line)) {
    if (line.length() == 0 ||line[0] == '#') continue;
    int tok1 = line.find('$');
    int tok2 = line.find('=', tok1);
    std::string name = line.substr(0, tok1);
    std::string setting = line.substr(tok1 + 1, (tok2 - tok1 - 1));
    std::string value = line.substr(tok2 + 1);
    Site * site = getSite(name);
    if (site == NULL) site = sites[name] = new Site(name);
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
      site->setMaxLogins(strToInt(value));
    }
    else if (!setting.compare("maxdn")) {
      site->setMaxDn(strToInt(value));
    }
    else if (!setting.compare("maxup")) {
      site->setMaxUp(strToInt(value));
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
      int avgspeed = strToInt(value.substr(split + 1));
      site->setAverageSpeed(sitename, avgspeed);
    }
  }
  sitedbfile.close();
}

void SiteManager::writeDataFile() {
  std::ofstream sitedbfile;
  sitedbfile.open (SITEDB, std::ios::trunc);
  sitedbfile << "# this is an example entry\n#SITENAME$addr=123.345.567.789\n#SITENAME$port=1337\n#SITENAME$user=yourusername\n#SITENAME$pass=yourpassword\n#SITENAME$logins=3\n#SITENAME$maxup=2\n#SITENAME$maxdn=1\n#SITENAME$section=TV-X264$/tv-720p\n#SITENAME$section=GAMES$/games\n" << std::endl;
  std::map<std::string, Site *>::iterator it;
  for (it = sites.begin(); it != sites.end(); it++) {
    Site * site = it->second;
    std::string name = it->second->getName();
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
}

Site * SiteManager::getSite(std::string name) {
  std::map<std::string, Site *>::iterator it = sites.find(name);
  if (it == sites.end()) return NULL;
  else return (*it).second;
}

int SiteManager::strToInt(std::string str) {
  int num;
  std::istringstream ss(str);
  ss >> num;
  return num;
}
