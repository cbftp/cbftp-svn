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
  binary = false;
  listcommand = SITE_LIST_STAT;
  ssltransfer = SITE_SSL_PREFER_OFF;
  sslconn = true;
  cpsvsupported = true;
  brokenpasv = false;
  allowdownload = true;
  allowupload = true;
  proxytype = SITE_PROXY_GLOBAL;
  proxyname = "";
  rank = SITE_RANK_USE_GLOBAL;
  ranktolerance = SITE_RANK_USE_GLOBAL;
}

std::map<std::string, std::string>::const_iterator Site::sectionsBegin() const {
  return sections.begin();
}

std::map<std::string, std::string>::const_iterator Site::sectionsEnd() const {
  return sections.end();
}

std::map<std::string, int>::const_iterator Site::avgspeedBegin() const {
  return avgspeed.begin();
}

std::map<std::string, int>::const_iterator Site::avgspeedEnd() const {
  return avgspeed.end();
}

unsigned int Site::getMaxLogins() const {
  if (logins == 0) {
    return REPORT_LOGINS_IF_UNLIMITED;
  }
  return logins;
}

unsigned int Site::getMaxUp() const {
  if (max_up == 0) {
    return getMaxLogins();
  }
  return max_up;
}

unsigned int Site::getMaxDown() const {
  if (max_dn == 0) {
    return getMaxLogins();
  }
  return max_dn;
}

unsigned int Site::getMaxIdleTime() const {
  return max_idletime;
}

unsigned int Site::getInternMaxLogins() const {
  return logins;
}

unsigned int Site::getInternMaxUp() const {
  return max_up;
}

unsigned int Site::getInternMaxDown() const {
  return max_dn;
}

std::string Site::getBasePath() const {
  return basepath;
}

bool Site::unlimitedLogins() const {
  return logins == 0;
}

bool Site::unlimitedUp() const {
  return max_up == 0;
}

bool Site::unlimitedDown() const {
  return max_dn == 0;
}

int Site::getAverageSpeed(std::string target) const {
  std::map<std::string, int>::const_iterator it = avgspeed.find(target);
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

bool Site::needsPRET() const {
  return pret;
}

void Site::setPRET(bool val) {
  pret = val;
}

bool Site::forceBinaryMode() const {
  return binary;
}

void Site::setForceBinaryMode(bool val) {
  binary = val;
}

bool Site::SSL() const {
  return sslconn;
}

int Site::getSSLTransferPolicy() const {
  return ssltransfer;
}

void Site::setSSLTransferPolicy(int policy) {
  ssltransfer = policy;
}

bool Site::supportsCPSV() const {
  return cpsvsupported;
}

void Site::setSupportsCPSV(bool supported) {
  cpsvsupported = supported;
}

int Site::getListCommand() const {
  return listcommand;
}

void Site::setListCommand(int command) {
  listcommand = command;
}

void Site::setSSL(bool val) {
  sslconn = val;
}

bool Site::getAllowUpload() const {
  return allowupload;
}

bool Site::getAllowDownload() const {
  return allowdownload;
}

int Site::getProxyType() const {
  return proxytype;
}

std::string Site::getProxy() const {
  return proxyname;
}

void Site::setAllowUpload(bool val) {
  allowupload = val;
}

void Site::setAllowDownload(bool val) {
  allowdownload = val;
}

bool Site::hasBrokenPASV() const {
  return brokenpasv;
}

void Site::setBrokenPASV(bool val) {
  brokenpasv = val;
}

std::string Site::getName() const {
  return name;
}

std::string Site::getSectionPath(std::string sectionname) const {
  std::map<std::string, std::string>::const_iterator it = sections.find(sectionname);
  if (it == sections.end()) return "/";
  return it->second;
}

bool Site::hasSection(std::string sectionname) const {
  std::map<std::string, std::string>::const_iterator it = sections.find(sectionname);
  if (it == sections.end()) return false;
  return true;
}

std::string Site::getAddress() const {
  return address;
}

std::string Site::getPort() const {
  return port;
}

std::string Site::getUser() const {
  return user;
}

std::string Site::getPass() const {
  return pass;
}

int Site::getRank() const {
  return rank;
}

int Site::getRankTolerance() const {
  return ranktolerance;
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

void Site::setRank(int rank) {
  this->rank = rank;
}

void Site::setRankTolerance(int tolerance) {
  ranktolerance = tolerance;
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

void Site::setProxyType(int proxytype) {
  this->proxytype = proxytype;
}

void Site::setProxy(std::string proxyname) {
  this->proxyname = proxyname;
}

void Site::clearSections() {
  sections.clear();
}

void Site::addSection(std::string name, std::string path) {
  sections[name] = path;
}

bool Site::isAffiliated(const std::string & affil) const {
  if (affils.find(affil) != affils.end()) {
    return true;
  }
  return false;
}

void Site::addAffil(const std::string & affil) {
  affils[affil] = true;
}

void Site::clearAffils() {
  affils.clear();
}

bool Site::isBannedGroup(const std::string & group) const {
  if (bannedgroups.find(group) != bannedgroups.end()) {
    return true;
  }
  return false;
}

void Site::addBannedGroup(const std::string & group) {
  bannedgroups[group] = true;
}

void Site::clearBannedGroups() {
  bannedgroups.clear();
}

std::map<std::string, bool>::const_iterator Site::affilsBegin() const {
  return affils.begin();
}

std::map<std::string, bool>::const_iterator Site::affilsEnd() const {
  return affils.end();
}

std::map<std::string, bool>::const_iterator Site::bannedGroupsBegin() const {
  return bannedgroups.begin();
}

std::map<std::string, bool>::const_iterator Site::bannedGroupsEnd() const {
  return bannedgroups.end();
}

std::list<std::string> Site::getSectionsForPath(std::string path) const {
  std::map<std::string, std::string>::const_iterator it;
  std::list<std::string> retsections;
  for (it = sections.begin(); it!= sections.end(); it++) {
    if (it->second == path) {
      retsections.push_back(it->first);
    }
  }
  return retsections;
}

std::list<std::string> Site::getSectionsForPartialPath(std::string path) const {
  std::map<std::string, std::string>::const_iterator it;
  std::list<std::string> retsections;
  for (it = sections.begin(); it!= sections.end(); it++) {
    if (path.find(it->second) != std::string::npos) {
      if (path.length() > it->second.length() && path[it->second.length()] != '/') {
        continue;
      }
      retsections.push_back(it->first);
    }
  }
  return retsections;
}
