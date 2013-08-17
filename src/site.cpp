#include "site.h"

Site::Site() {

}

Site::Site(std::string name) {
  this->name = name;
  address = "ftp.sunet.se";
  port = "21";
  user = "anonymous";
  pass = "anonymous";
  basepath = "/";
  logins = 0;
  max_up = 0;
  max_dn = 0;
  max_idletime = 60;
  pret = false;
  sslfxpforced = false;
  sslconn = true;
  brokenpasv = false;
  allowdownload = true;
  allowupload = true;
}

void Site::copy(Site * site) {
  this->name = site->name;
  this->address = site->address;
  this->port = site->port;
  this->user = site->user;
  this->pass = site->pass;
  this->logins = site->logins;
  this->max_up = site->max_up;
  this->max_dn = site->max_dn;
  this->pret = site->pret;
  this->sslfxpforced = site->sslfxpforced;
  this->sslconn = site->sslconn;
  this->brokenpasv = site->brokenpasv;
  this->sections = site->sections;
  this->avgspeed = site->avgspeed;
  this->allowupload = site->allowupload;
  this->allowdownload = site->allowdownload;
  this->affils = site->affils;
  this->basepath = site->basepath;
}

std::map<std::string, std::string>::iterator Site::sectionsBegin() {
  return sections.begin();
}

std::map<std::string, std::string>::iterator Site::sectionsEnd() {
  return sections.end();
}

std::map<std::string, int>::iterator Site::avgspeedBegin() {
  return avgspeed.begin();
}

std::map<std::string, int>::iterator Site::avgspeedEnd() {
  return avgspeed.end();
}

unsigned int Site::getMaxLogins() {
  if (logins == 0) {
    return REPORT_LOGINS_IF_UNLIMITED;
  }
  return logins;
}

unsigned int Site::getMaxUp() {
  if (max_up == 0) {
    return getMaxLogins();
  }
  return max_up;
}

unsigned int Site::getMaxDown() {
  if (max_dn == 0) {
    return getMaxLogins();
  }
  return max_dn;
}

unsigned int Site::getMaxIdleTime() {
  return max_idletime;
}

unsigned int Site::getInternMaxLogins() {
  return logins;
}

unsigned int Site::getInternMaxUp() {
  return max_up;
}

unsigned int Site::getInternMaxDown() {
  return max_dn;
}

std::string Site::getBasePath() {
  return basepath;
}

bool Site::unlimitedLogins() {
  return logins == 0;
}

bool Site::unlimitedUp() {
  return max_up == 0;
}

bool Site::unlimitedDown() {
  return max_dn == 0;
}

int Site::getAverageSpeed(std::string target) {
  std::map<std::string, int>::iterator it = avgspeed.find(target);
  if (it == avgspeed.end()) return 1024;
  return it->second;
}

void Site::setAverageSpeed(std::string target, int speed) {
  std::map<std::string, int>::iterator it = avgspeed.find(target);
  if (it != avgspeed.end()) avgspeed.erase(it);
  avgspeed[target] = speed;
}

void Site::pushTransferSpeed(std::string target, int speed) {
  std::map<std::string, int>::iterator it = avgspeed.find(target);
  int oldspeed;
  if (it == avgspeed.end()) oldspeed = 1024;
  else {
    oldspeed = it->second;
    avgspeed.erase(it);
  }
  avgspeed[target] = (int) ((speed / 5) + (oldspeed * 0.8));
}

bool Site::needsPRET() {
  return pret;
}

void Site::setPRET(bool val) {
  pret = val;
}

bool Site::SSL() {
  return sslconn;
}

bool Site::SSLFXPForced() {
  return sslfxpforced;
}

void Site::setSSLFXPForced(bool val) {
  sslfxpforced = val;
}

void Site::setSSL(bool val) {
  sslconn = val;
}

bool Site::getAllowUpload() {
  return allowupload;
}

bool Site::getAllowDownload() {
  return allowdownload;
}

void Site::setAllowUpload(bool val) {
  allowupload = val;
}

void Site::setAllowDownload(bool val) {
  allowdownload = val;
}

bool Site::hasBrokenPASV() {
  return brokenpasv;
}

void Site::setBrokenPASV(bool val) {
  brokenpasv = val;
}

std::string Site::getName() {
  return name;
}

std::string Site::getSectionPath(std::string sectionname) {
  std::map<std::string, std::string>::iterator it = sections.find(sectionname);
  if (it == sections.end()) return "/";
  return it->second;
}

bool Site::hasSection(std::string sectionname) {
  std::map<std::string, std::string>::iterator it = sections.find(sectionname);
  if (it == sections.end()) return false;
  return true;
}

std::string Site::getAddress() {
  return address;
}

std::string Site::getPort() {
  return port;
}

std::string Site::getUser() {
  return user;
}

std::string Site::getPass() {
  return pass;
}

void Site::setName(std::string name) {
  this->name = name;
}

void Site::setAddress(std::string addr) {
  address = addr;
}

void Site::setPort(std::string port) {
  this->port = port;
}

void Site::setUser(std::string user) {
  this->user = user;
}

void Site::setPass(std::string pass) {
  this->pass = pass;
}

void Site::setBasePath(std::string basepath) {
  this->basepath = basepath;
}

void Site::setMaxLogins(unsigned int num) {
  logins = num;
  if (num > 0 && max_dn > 0 && max_dn < num) {
    max_dn = num;
  }
  if (num > 0 && max_up > 0 && max_up < num) {
    max_up = num;
  }
}

void Site::setMaxDn(unsigned int num) {
  max_dn = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxUp(unsigned int num) {
  max_up = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxIdleTime(unsigned int idletime) {
  max_idletime = idletime;
}

void Site::clearSections() {
  sections.clear();
}

void Site::addSection(std::string name, std::string path) {
  sections[name] = path;
}

void Site::clearAffils() {
  affils.clear();
}

bool Site::isAffiliated(std::string affil) {
  if (affils.find(affil) != affils.end()) {
    return true;
  }
  return false;
}

void Site::addAffil(std::string affil) {
  affils[affil] = true;
}

std::map<std::string, bool>::iterator Site::affilsBegin() {
  return affils.begin();
}

std::map<std::string, bool>::iterator Site::affilsEnd() {
  return affils.end();
}

std::list<std::string> Site::getSectionsForPath(std::string path) {
  std::map<std::string, std::string>::iterator it;
  std::list<std::string> retsections;
  for (it = sections.begin(); it!= sections.end(); it++) {
    if (it->second == path) {
      retsections.push_back(it->first);
    }
  }
  return retsections;
}
