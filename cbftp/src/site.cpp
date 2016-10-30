#include "site.h"

#include "path.h"
#include "util.h"

Site::Site() {

}

Site::Site(const std::string & name) :
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
  disabled(false),
  allowupload(true),
  allowdownload(true),
  priority(SITE_PRIORITY_NORMAL),
  proxytype(SITE_PROXY_GLOBAL),
  transfersourcepolicy(SITE_TRANSFER_POLICY_ALLOW),
  transfertargetpolicy(SITE_TRANSFER_POLICY_ALLOW),
  aggressivemkdir(false)
{
  addresses.push_back(std::pair<std::string, std::string>("ftp.sunet.se", "21"));
}

std::map<std::string, Path>::const_iterator Site::sectionsBegin() const {
  return sections.begin();
}

std::map<std::string, Path>::const_iterator Site::sectionsEnd() const {
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

const Path & Site::getBasePath() const {
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

bool Site::getDisabled() const {
  return disabled;
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

void Site::setDisabled(bool val) {
  disabled = val;
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

bool Site::getAggressiveMkdir() const {
  return aggressivemkdir;
}

void Site::setAggressiveMkdir(bool aggressive) {
  aggressivemkdir = aggressive;
}

const Path Site::getSectionPath(const std::string & sectionname) const {
  std::map<std::string, Path>::const_iterator it = sections.find(sectionname);
  if (it == sections.end()) return "/";
  return it->second;
}

bool Site::hasSection(const std::string & sectionname) const {
  std::map<std::string, Path>::const_iterator it = sections.find(sectionname);
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

int Site::getTransferSourcePolicy() const {
  return transfersourcepolicy;
}

int Site::getTransferTargetPolicy() const {
  return transfertargetpolicy;
}

void Site::setName(const std::string & name) {
  this->name = name;
}

void Site::setAddresses(std::string addrports) {
  addresses.clear();
  size_t pos;
  while ((pos = addrports.find(";")) != std::string::npos) addrports[pos] = ' ';
  while ((pos = addrports.find(",")) != std::string::npos) addrports[pos] = ' ';
  std::list<std::string> addrpairs = util::split(addrports);
  for (std::list<std::string>::const_iterator it = addrpairs.begin(); it != addrpairs.end(); it++) {
    size_t splitpos = it->find(":");
    std::string addr = *it;
    std::string port = "21";
    if (splitpos != std::string::npos) {
      addr = it->substr(0, splitpos);
      port = it->substr(splitpos + 1);
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
  if (affilslower.find(util::toLower(affil)) != affilslower.end()) {
    return true;
  }
  return false;
}

void Site::addAffil(const std::string & affil) {
  affils.insert(affil);
  affilslower[util::toLower(affil)] = true;
}

void Site::clearAffils() {
  affils.clear();
  affilslower.clear();
}

bool Site::isBannedGroup(const std::string & section, const std::string & group) const {
  std::string sectionlow = util::toLower(section);
  std::string grouplow = util::toLower(group);
  std::map<std::string, std::set<std::string> >::const_iterator it;
  if ((it = bannedgroupssectionexcept.find(sectionlow)) != bannedgroupssectionexcept.end()) {
    return !it->second.count(grouplow);
  }
  if ((it = bannedgroupssectionexcept.find("")) != bannedgroupssectionexcept.end()) {
    return !it->second.count(grouplow);
  }
  return bannedgroupslower.count(grouplow) || bannedgroupslower.count(sectionlow + "/" + grouplow);
}

void Site::addBannedGroup(const std::string & group) {
  if (!group.length()) {
    return;
  }
  bannedgroups.insert(group);
  std::string groupstr = util::toLower(group);
  if (groupstr[0] != '!') {
    bannedgroupslower[groupstr] = true;
    return;
  }
  groupstr = groupstr.substr(1);
  size_t slashpos = groupstr.find("/");
  std::string section;
  if (slashpos != std::string::npos) {
    section = groupstr.substr(0, slashpos);
    groupstr = groupstr.substr(slashpos + 1);
  }
  if (!bannedgroupssectionexcept.count(section)) {
    bannedgroupssectionexcept[section] = std::set<std::string>();
  }
  bannedgroupssectionexcept[section].insert(groupstr);
}

void Site::clearBannedGroups() {
  bannedgroups.clear();
  bannedgroupslower.clear();
  bannedgroupssectionexcept.clear();
}

void Site::setTransferSourcePolicy(int policy) {
  transfersourcepolicy = policy;
}

void Site::setTransferTargetPolicy(int policy) {
  transfertargetpolicy = policy;
}

void Site::addAllowedSourceSite(const Pointer<Site> & site) {
  if (transfersourcepolicy == SITE_TRANSFER_POLICY_BLOCK) {
    exceptsourcesites.insert(site);
  }
  else {
    exceptsourcesites.erase(site);
  }
}

void Site::addBlockedSourceSite(const Pointer<Site> & site) {
  if (transfersourcepolicy == SITE_TRANSFER_POLICY_ALLOW) {
    exceptsourcesites.insert(site);
  }
  else {
    exceptsourcesites.erase(site);
  }
}

void Site::addAllowedTargetSite(const Pointer<Site> & site) {
  if (transfertargetpolicy == SITE_TRANSFER_POLICY_BLOCK) {
    excepttargetsites.insert(site);
  }
  else {
    excepttargetsites.erase(site);
  }
}

void Site::addBlockedTargetSite(const Pointer<Site> & site) {
  if (transfertargetpolicy == SITE_TRANSFER_POLICY_ALLOW) {
    excepttargetsites.insert(site);
  }
  else {
    excepttargetsites.erase(site);
  }
}

void Site::addExceptSourceSite(const Pointer<Site> & site) {
  exceptsourcesites.insert(site);
}

void Site::addExceptTargetSite(const Pointer<Site> & site) {
  excepttargetsites.insert(site);
}

void Site::removeExceptSite(const Pointer<Site> & site) {
  exceptsourcesites.erase(site);
  excepttargetsites.erase(site);
}

void Site::clearExceptSites() {
  exceptsourcesites.clear();
  excepttargetsites.clear();
}

bool Site::isAllowedTargetSite(const Pointer<Site> & site) const {
  std::set<Pointer<Site> >::const_iterator it = excepttargetsites.find(site);
  if (it != excepttargetsites.end()) {
    return transfertargetpolicy == SITE_TRANSFER_POLICY_BLOCK;
  }
  return transfertargetpolicy == SITE_TRANSFER_POLICY_ALLOW;
}

std::set<std::string>::const_iterator Site::affilsBegin() const {
  return affils.begin();
}

std::set<std::string>::const_iterator Site::affilsEnd() const {
  return affils.end();
}

std::set<std::string>::const_iterator Site::bannedGroupsBegin() const {
  return bannedgroups.begin();
}

std::set<std::string>::const_iterator Site::bannedGroupsEnd() const {
  return bannedgroups.end();
}

std::set<Pointer<Site> >::const_iterator Site::exceptSourceSitesBegin() const {
  return exceptsourcesites.begin();
}

std::set<Pointer<Site> >::const_iterator Site::exceptSourceSitesEnd() const {
  return exceptsourcesites.end();
}

std::set<Pointer<Site> >::const_iterator Site::exceptTargetSitesBegin() const {
  return excepttargetsites.begin();
}

std::set<Pointer<Site> >::const_iterator Site::exceptTargetSitesEnd() const {
  return excepttargetsites.end();
}

std::list<std::string> Site::getSectionsForPath(const Path & path) const {
  std::map<std::string, Path>::const_iterator it;
  std::list<std::string> retsections;
  for (it = sections.begin(); it!= sections.end(); it++) {
    if (it->second == path) {
      retsections.push_back(it->first);
    }
  }
  return retsections;
}

std::list<std::string> Site::getSectionsForPartialPath(const Path & path) const {
  std::map<std::string, Path>::const_iterator it;
  std::list<std::string> retsections;
  for (it = sections.begin(); it!= sections.end(); it++) {
    if (path.contains(it->second)) {
      retsections.push_back(it->first);
    }
  }
  return retsections;
}

std::pair<Path, Path> Site::splitPathInSectionAndSubpath(const Path & path) const {
  Path sectionpath;
  std::list<std::string> sections = getSectionsForPartialPath(path);
  if (sections.size()) {
    sectionpath = getSectionPath(sections.front());
  }
  Path subpath = path - sectionpath;
  return std::pair<Path, Path>(sectionpath, subpath);
}
