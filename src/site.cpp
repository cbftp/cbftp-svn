#include "site.h"

#include "path.h"
#include "util.h"
#include "globalcontext.h"
#include "sectionmanager.h"

Site::Site() : skiplist(global->getSkipList()) {

}

Site::Site(const std::string & name) :
  name(name),
  user("anonymous"),
  pass("anonymous"),
  basepath("/"),
  logins(0),
  maxup(0),
  maxdn(0),
  maxdnpre(0),
  maxdncomplete(0),
  maxdntransferjob(0),
  maxidletime(60),
  pret(false),
  binary(false),
  listcommand(SITE_LIST_STAT),
  tlsmode(TLSMode::AUTH_TLS),
  ssltransfer(SITE_SSL_PREFER_OFF),
  sscnsupported(true),
  cpsvsupported(true),
  brokenpasv(false),
  disabled(false),
  allowupload(SITE_ALLOW_TRANSFER_YES),
  allowdownload(SITE_ALLOW_TRANSFER_YES),
  priority(SitePriority::NORMAL),
  xdupe(true),
  proxytype(SITE_PROXY_GLOBAL),
  transfersourcepolicy(SITE_TRANSFER_POLICY_ALLOW),
  transfertargetpolicy(SITE_TRANSFER_POLICY_ALLOW),
  skiplist(global->getSkipList()),
  freeslot(false)
{
  addresses.push_back(std::pair<std::string, std::string>("ftp.sunet.se", "21"));
}

Site::Site(const Site & other) {
  name = other.name;
  addresses = other.addresses;
  user = other.user;
  pass = other.pass;
  basepath = other.basepath;
  logins = other.logins;
  maxup = other.maxup;
  maxdn = other.maxdn;
  maxdnpre = other.maxdnpre;
  maxdncomplete = other.maxdncomplete;
  maxdntransferjob = other.maxdntransferjob;
  freeslot = other.freeslot;
  maxidletime = other.maxidletime;
  pret = other.pret;
  binary = other.binary;
  listcommand = other.listcommand;
  tlsmode = other.tlsmode;
  ssltransfer = other.ssltransfer;
  sscnsupported = other.sscnsupported;
  cpsvsupported = other.cpsvsupported;
  brokenpasv = other.brokenpasv;
  disabled = other.disabled;
  allowupload = other.allowupload;
  allowdownload = other.allowdownload;
  priority = other.priority;
  xdupe = other.xdupe;
  sections = other.sections;
  affils = other.affils;
  affilslower = other.affilslower;
  proxytype = other.proxytype;
  transfersourcepolicy = other.transfersourcepolicy;
  transfertargetpolicy = other.transfertargetpolicy;
  skiplist = other.skiplist;
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
  if (maxup == 0) {
    return getMaxLogins();
  }
  return maxup;
}

unsigned int Site::getMaxDown() const {
  if (maxdn == 0) {
    return getMaxLogins();
  }
  return maxdn;
}

unsigned int Site::getMaxDownPre() const {
  if (maxdnpre == 0) {
    return getMaxDown();
  }
  return maxdnpre;
}

unsigned int Site::getMaxDownComplete() const {
  if (maxdncomplete == 0) {
    return getMaxDown();
  }
  return maxdncomplete;
}

unsigned int Site::getMaxDownTransferJob() const {
  if (maxdntransferjob == 0) {
    return getMaxDown();
  }
  return maxdntransferjob;
}

unsigned int Site::getMaxIdleTime() const {
  return maxidletime;
}

unsigned int Site::getInternMaxLogins() const {
  return logins;
}

unsigned int Site::getInternMaxUp() const {
  return maxup;
}

unsigned int Site::getInternMaxDown() const {
  return maxdn;
}

unsigned int Site::getInternMaxDownPre() const {
  return maxdnpre;
}

unsigned int Site::getInternMaxDownComplete() const {
  return maxdncomplete;
}

unsigned int Site::getInternMaxDownTransferJob() const {
  return maxdntransferjob;
}

bool Site::getLeaveFreeSlot() const {
  return freeslot;
}

const Path & Site::getBasePath() const {
  return basepath;
}

bool Site::unlimitedLogins() const {
  return logins == 0;
}

bool Site::unlimitedUp() const {
  return maxup == 0;
}

bool Site::unlimitedDown() const {
  return maxdn == 0;
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

TLSMode Site::getTLSMode() const {
  return tlsmode;
}

int Site::getSSLTransferPolicy() const {
  return ssltransfer;
}

void Site::setSSLTransferPolicy(int policy) {
  ssltransfer = policy;
}

SitePriority Site::getPriority() const {
  return priority;
}

std::string Site::getPriorityText(SitePriority priority) {
  switch (priority) {
    case SitePriority::VERY_LOW:
      return "Very low";
    case SitePriority::LOW:
      return "Low";
    case SitePriority::NORMAL:
      return "Normal";
    case SitePriority::HIGH:
      return "High";
    case SitePriority::VERY_HIGH:
      return "Very high";
  }
  return "Unknown";
}

void Site::setPriority(SitePriority priority) {
  this->priority = priority;
}

bool Site::supportsSSCN() const {
  return sscnsupported;
}

bool Site::supportsCPSV() const {
  return cpsvsupported;
}

void Site::setSupportsSSCN(bool supported) {
  sscnsupported = supported;
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

void Site::setTLSMode(TLSMode mode) {
  tlsmode = mode;
}

bool Site::getDisabled() const {
  return disabled;
}

SiteAllowTransfer Site::getAllowUpload() const {
  return allowupload;
}

SiteAllowTransfer Site::getAllowDownload() const {
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

void Site::setAllowUpload(SiteAllowTransfer val) {
  allowupload = val;
}

void Site::setAllowDownload(SiteAllowTransfer val) {
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

unsigned int Site::sectionsSize() const {
  return sections.size();
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
  if (!addresses.size()) {
    return "";
  }
  return addresses.front().first;
}

std::string Site::getPort() const {
  if (!addresses.size()) {
    return "0";
  }
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

bool Site::useXDUPE() const {
  return xdupe;
}

SkipList & Site::getSkipList() {
  return skiplist;
}

const std::map<std::string, Path> & Site::getSections() const {
  return sections;
}

void Site::setName(const std::string & name) {
  this->name = name;
}

void Site::setAddresses(std::string addrports) {
  addresses.clear();
  size_t pos;
  while ((pos = addrports.find(";")) != std::string::npos) addrports[pos] = ' ';
  while ((pos = addrports.find(",")) != std::string::npos) addrports[pos] = ' ';
  std::list<std::string> addrpairs = util::trim(util::split(addrports));
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
  if (num > 0 && maxdn > num) {
    maxdn = num;
  }
  if (num > 0 && maxdnpre > num) {
    maxdnpre = num;
  }
  if (num > 0 && maxdncomplete > num) {
    maxdncomplete = num;
  }
  if (num > 0 && maxdntransferjob > num) {
    maxdntransferjob = num;
  }
  if (num > 0 && maxup > num) {
    maxup = num;
  }
}

void Site::setMaxDn(unsigned int num) {
  maxdn = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxUp(unsigned int num) {
  maxup = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxDnPre(unsigned int num) {
  maxdnpre = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxDnComplete(unsigned int num) {
  maxdncomplete = num > logins && logins > 0 ? logins : num;
}

void Site::setMaxDnTransferJob(unsigned int num) {
  maxdntransferjob = num > logins && logins > 0 ? logins : num;
}

void Site::setLeaveFreeSlot(bool free) {
  freeslot = free;
}

void Site::setMaxIdleTime(unsigned int idletime) {
  maxidletime = idletime;
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
  global->getSectionManager()->addSection(name);
}

void Site::renameSection(const std::string & oldname, const std::string & newname) {
  sections[newname] = sections[oldname];
  sections.erase(oldname);
}

void Site::removeSection(const std::string & name) {
  sections.erase(name);
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

void Site::setTransferSourcePolicy(int policy) {
  transfersourcepolicy = policy;
}

void Site::setTransferTargetPolicy(int policy) {
  transfertargetpolicy = policy;
}

void Site::setUseXDUPE(bool xdupe) {
  this->xdupe = xdupe;
}

void Site::addAllowedSourceSite(const std::shared_ptr<Site> & site) {
  if (transfersourcepolicy == SITE_TRANSFER_POLICY_BLOCK) {
    exceptsourcesites.insert(site);
  }
  else {
    exceptsourcesites.erase(site);
  }
}

void Site::addBlockedSourceSite(const std::shared_ptr<Site> & site) {
  if (transfersourcepolicy == SITE_TRANSFER_POLICY_ALLOW) {
    exceptsourcesites.insert(site);
  }
  else {
    exceptsourcesites.erase(site);
  }
}

void Site::addAllowedTargetSite(const std::shared_ptr<Site> & site) {
  if (transfertargetpolicy == SITE_TRANSFER_POLICY_BLOCK) {
    excepttargetsites.insert(site);
  }
  else {
    excepttargetsites.erase(site);
  }
}

void Site::addBlockedTargetSite(const std::shared_ptr<Site> & site) {
  if (transfertargetpolicy == SITE_TRANSFER_POLICY_ALLOW) {
    excepttargetsites.insert(site);
  }
  else {
    excepttargetsites.erase(site);
  }
}

void Site::addExceptSourceSite(const std::shared_ptr<Site> & site) {
  exceptsourcesites.insert(site);
}

void Site::addExceptTargetSite(const std::shared_ptr<Site> & site) {
  excepttargetsites.insert(site);
}

void Site::removeExceptSite(const std::shared_ptr<Site> & site) {
  exceptsourcesites.erase(site);
  excepttargetsites.erase(site);
}

void Site::clearExceptSites() {
  exceptsourcesites.clear();
  excepttargetsites.clear();
}

bool Site::isAllowedTargetSite(const std::shared_ptr<Site> & site) const {
  std::set<std::shared_ptr<Site> >::const_iterator it = excepttargetsites.find(site);
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

std::set<std::shared_ptr<Site> >::const_iterator Site::exceptSourceSitesBegin() const {
  return exceptsourcesites.begin();
}

std::set<std::shared_ptr<Site> >::const_iterator Site::exceptSourceSitesEnd() const {
  return exceptsourcesites.end();
}

std::set<std::shared_ptr<Site> >::const_iterator Site::exceptTargetSitesBegin() const {
  return excepttargetsites.begin();
}

std::set<std::shared_ptr<Site> >::const_iterator Site::exceptTargetSitesEnd() const {
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
  Path sectionpath("/");
  std::list<std::string> sections = getSectionsForPartialPath(path);
  for (std::list<std::string>::const_iterator it = sections.begin(); it != sections.end(); ++it) {
    Path currentsectionpath = getSectionPath(*it);
    if (currentsectionpath.toString().length() > sectionpath.toString().length()) {
      sectionpath = currentsectionpath;
    }
  }
  Path subpath = path - sectionpath;
  return std::pair<Path, Path>(sectionpath, subpath);
}

void Site::addTransferStatsFile(StatsDirection direction, const std::string & other, unsigned long long int size) {
  if (direction == STATS_DOWN) {
    if (sitessizedown.find(other) == sitessizedown.end()) {
      sitessizedown[other] = HourlyAllTracking();
      sitesfilesdown[other] = HourlyAllTracking();
    }
    sitessizedown[other].add(size);
    sitesfilesdown[other].add(1);
  }
  else {
    if (sitessizeup.find(other) == sitessizeup.end()) {
      sitessizeup[other] = HourlyAllTracking();
      sitesfilesup[other] = HourlyAllTracking();
    }
    sitessizeup[other].add(size);
    sitesfilesup[other].add(1);
  }
  addTransferStatsFile(direction, size);
}

void Site::addTransferStatsFile(StatsDirection direction, unsigned long long int size) {
  if (direction == STATS_DOWN) {
    sizedown.add(size);
    filesdown.add(1);
  }
  else {
    sizeup.add(size);
    filesup.add(1);
  }
}

void Site::tickMinute() {
  sizeup.tickMinute();
  filesup.tickMinute();
  sizedown.tickMinute();
  filesdown.tickMinute();
  for (std::map<std::string, HourlyAllTracking>::iterator it = sitessizeup.begin(); it != sitessizeup.end(); it++) {
    it->second.tickMinute();
  }
  for (std::map<std::string, HourlyAllTracking>::iterator it = sitesfilesup.begin(); it != sitesfilesup.end(); it++) {
    it->second.tickMinute();
  }
  for (std::map<std::string, HourlyAllTracking>::iterator it = sitessizedown.begin(); it != sitessizedown.end(); it++) {
    it->second.tickMinute();
  }
  for (std::map<std::string, HourlyAllTracking>::iterator it = sitesfilesdown.begin(); it != sitesfilesdown.end(); it++) {
    it->second.tickMinute();
  }
}

const HourlyAllTracking & Site::getSizeUp() const {
  return sizeup;
}

const HourlyAllTracking & Site::getSizeDown() const {
  return sizedown;
}

const HourlyAllTracking & Site::getFilesUp() const {
  return filesup;
}

const HourlyAllTracking & Site::getFilesDown() const {
  return filesdown;
}

HourlyAllTracking & Site::getSizeUp() {
  return sizeup;
}

HourlyAllTracking & Site::getSizeDown() {
  return sizedown;
}

HourlyAllTracking & Site::getFilesUp() {
  return filesup;
}

HourlyAllTracking & Site::getFilesDown() {
  return filesdown;
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::sizeUpBegin() const {
  return sitessizeup.begin();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::filesUpBegin() const {
  return sitesfilesup.begin();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::sizeDownBegin() const {
  return sitessizedown.begin();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::filesDownBegin() const {
  return sitesfilesdown.begin();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::sizeUpEnd() const {
  return sitessizeup.end();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::filesUpEnd() const {
  return sitesfilesup.end();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::sizeDownEnd() const {
  return sitessizedown.end();
}

std::map<std::string, HourlyAllTracking>::const_iterator Site::filesDownEnd() const {
  return sitesfilesdown.end();
}

HourlyAllTracking & Site::getSiteSizeUp(const std::string & site) {
  return sitessizeup[site];
}

HourlyAllTracking & Site::getSiteSizeDown(const std::string & site) {
  return sitessizedown[site];
}

HourlyAllTracking & Site::getSiteFilesUp(const std::string & site) {
  return sitesfilesup[site];
}

HourlyAllTracking & Site::getSiteFilesDown(const std::string & site) {
  return sitesfilesdown[site];
}

void Site::setSkipList(const SkipList & skiplist) {
  this->skiplist = skiplist;
}

void Site::setSections(const std::map<std::string, Path> & sections) {
  this->sections = sections;
}
