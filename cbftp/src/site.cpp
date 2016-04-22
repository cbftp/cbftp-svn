#include "site.h"

Site::Site() {

}

Site::Site(std::string name) :
  name(name),
  user("anonymous"),
  pass("anonymous"),
  basepath("/"),
  logins(0),
  max_up(0),
  max_dn(0),
  max_idletime(60),
  pret(false),
  binary(false),
  listcommand(SITE_LIST_STAT),
  sslconn(true),
  ssltransfer(SITE_SSL_PREFER_OFF),
  cpsvsupported(true),
  brokenpasv(false),
  allowupload(true),
  allowdownload(true),
  priority(SITE_PRIORITY_NORMAL),
  proxytype(SITE_PROXY_GLOBAL),
  rank(SITE_RANK_USE_GLOBAL),
  ranktolerance(SITE_RANK_USE_GLOBAL)
{
  addresses.push_back(std::pair<std::string, std::string>("ftp.sunet.se", "21"));
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

int Site::getAverageSpeed(const std::string & target) const {
  std::map<std::string, int>::const_iterator it = avgspeed.find(target);
  if (it == avgspeed.end()) return 1024;
  return it->second;
}

void Site::setAverageSpeed(const std::string & target, int speed) {
  std::map<std::string, int>::iterator it = avgspeed.find(target);
  if (it != avgspeed.end()) avgspeed.erase(it);
  avgspeed[target] = speed;
}

void Site::pushTransferSpeed(const std::string & target, int speed, unsigned long long int size) {
  std::map<std::string, int>::iterator it = avgspeed.find(target);
  int oldspeed;
  if (it == avgspeed.end()) {
    oldspeed = 1024;
    avgspeedsamples[target] = std::pair<int, unsigned long long int>(0, 0);
  }
  else {
    oldspeed = it->second;
    avgspeed.erase(it);
  }
  avgspeed[target] = (int) ((speed / 5) + (oldspeed * 0.8));
  ++avgspeedsamples[target].first;
  avgspeedsamples[target].second += size;
}

std::pair<int, unsigned long long int> Site::getAverageSpeedSamples(const std::string & target) const {
  std::map<std::string, std::pair<int, unsigned long long int> >::const_iterator it = avgspeedsamples.find(target);
  if (it != avgspeedsamples.end()) {
    return it->second;
  }
  return std::pair<int, unsigned long long int>(0, 0);
}

void Site::resetAverageSpeedSamples(const std::string & target) {
  std::map<std::string, std::pair<int, unsigned long long int> >::iterator it = avgspeedsamples.find(target);
  if (it != avgspeedsamples.end()) {
    it->second.first = 0;
    it->second.second = 0;
  }
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

int Site::getPriority() const {
  return priority;
}

void Site::setPriority(int priority) {
  this->priority = priority;
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

std::string Site::getSectionPath(const std::string & sectionname) const {
  std::map<std::string, std::string>::const_iterator it = sections.find(sectionname);
  if (it == sections.end()) return "/";
  return it->second;
}

bool Site::hasSection(const std::string & sectionname) const {
  std::map<std::string, std::string>::const_iterator it = sections.find(sectionname);
  if (it == sections.end()) return false;
  return true;
}

std::string Site::getAddress() const {
  return addresses.front().first;
}

std::string Site::getPort() const {
  return addresses.front().second;
}

std::list<std::pair<std::string, std::string> > Site::getAddresses() const {
  return addresses;
}

std::string Site::getAddressesAsString() const {
  std::string addressesconcat;
  for (std::list<std::pair<std::string, std::string> >::const_iterator it = addresses.begin(); it != addresses.end(); it++) {
    std::string addrport = it->first;
    if (it->second != "21") {
      addrport += ":" + it->second;
    }
    if (addressesconcat.length()) {
      addressesconcat += " ";
    }
    addressesconcat += addrport;
  }
  return addressesconcat;
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

void Site::setName(const std::string & name) {
  this->name = name;
}

void Site::setAddresses(std::string addrports) {
  addresses.clear();
  addrports += " ";
  size_t pos;
  while ((pos = addrports.find(";")) != std::string::npos) addrports[pos] = ' ';
  while ((pos = addrports.find(",")) != std::string::npos) addrports[pos] = ' ';
  while ((pos = addrports.find(" ")) != std::string::npos) {
    if (addrports[0] == ' ') {
      addrports = addrports.substr(1);
      continue;
    }
    std::string addrport = addrports.substr(0, pos);
    addrports = addrports.substr(pos + 1);
    size_t splitpos = addrport.find(":");
    std::string addr = addrport;
    std::string port = "21";
    if (splitpos != std::string::npos) {
      addr = addrport.substr(0, splitpos);
      port = addrport.substr(splitpos + 1);
    }
    addresses.push_back(std::pair<std::string, std::string>(addr, port));
  }
}

void Site::setPrimaryAddress(const std::string & addr, const std::string & port) {
  std::list<std::pair<std::string, std::string> >::iterator it;
  for (it = addresses.begin(); it != addresses.end(); it++) {
    if (it->first == addr && it->second == port) {
      addresses.erase(it);
      break;
    }
  }
  addresses.push_front(std::pair<std::string, std::string>(addr, port));
}

void Site::setUser(const std::string & user) {
  this->user = user;
}

void Site::setPass(const std::string & pass) {
  this->pass = pass;
}

void Site::setRank(int rank) {
  this->rank = rank;
}

void Site::setRankTolerance(int tolerance) {
  ranktolerance = tolerance;
}

void Site::setBasePath(const std::string & basepath) {
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

void Site::setProxy(const std::string & proxyname) {
  this->proxyname = proxyname;
}

void Site::clearSections() {
  sections.clear();
}

void Site::addSection(const std::string & name, const std::string & path) {
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

std::list<std::string> Site::getSectionsForPath(const std::string & path) const {
  std::map<std::string, std::string>::const_iterator it;
  std::list<std::string> retsections;
  for (it = sections.begin(); it!= sections.end(); it++) {
    if (it->second == path) {
      retsections.push_back(it->first);
    }
  }
  return retsections;
}

std::list<std::string> Site::getSectionsForPartialPath(const std::string & path) const {
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
