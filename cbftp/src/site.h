#pragma once

#include <list>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "path.h"
#include "skiplist.h"
#include "statistics.h"
#include "hourlyalltracking.h"

#define REPORT_LOGINS_IF_UNLIMITED 10

#define SITE_PROXY_GLOBAL 820
#define SITE_PROXY_NONE 821
#define SITE_PROXY_USE 822

#define SITE_SSL_ALWAYS_OFF 830
#define SITE_SSL_PREFER_OFF 831
#define SITE_SSL_PREFER_ON 832
#define SITE_SSL_ALWAYS_ON 833

#define SITE_LIST_STAT 840
#define SITE_LIST_LIST 841

enum class SitePriority {
  VERY_LOW = 711,
  LOW = 712,
  NORMAL = 713,
  HIGH = 714,
  VERY_HIGH = 715
};

#define SITE_TRANSFER_POLICY_ALLOW 817
#define SITE_TRANSFER_POLICY_BLOCK 818

enum SiteAllowTransfer {
  SITE_ALLOW_TRANSFER_NO = 821,
  SITE_ALLOW_TRANSFER_YES = 822,
  SITE_ALLOW_DOWNLOAD_MATCH_ONLY = 823
};

enum class TLSMode {
  NONE = 0,
  AUTH_TLS = 1,
  IMPLICIT = 2
};

class Site {
private:
  std::string name;
  std::list<std::pair<std::string, std::string> > addresses;
  std::string user;
  std::string pass;
  Path basepath;
  unsigned int logins;
  unsigned int maxup;
  unsigned int maxdn;
  unsigned int maxdnpre;
  unsigned int maxdncomplete;
  unsigned int maxdntransferjob;
  unsigned int maxidletime;
  bool pret;
  bool binary;
  int listcommand;
  TLSMode tlsmode;
  int ssltransfer;
  bool sscnsupported;
  bool cpsvsupported;
  bool brokenpasv;
  bool disabled;
  SiteAllowTransfer allowupload;
  SiteAllowTransfer allowdownload;
  SitePriority priority;
  bool xdupe;
  std::map<std::string, Path> sections;
  std::map<std::string, int> avgspeed;
  std::map<std::string, std::pair<int, unsigned long long int> > avgspeedsamples;
  std::set<std::string> affils;
  std::map<std::string, std::string> affilslower;
  std::set<std::shared_ptr<Site> > exceptsourcesites;
  std::set<std::shared_ptr<Site> > excepttargetsites;
  int proxytype;
  std::string proxyname;
  int transfersourcepolicy;
  int transfertargetpolicy;
  SkipList skiplist;
  std::map<std::string, HourlyAllTracking> sitessizeup;
  std::map<std::string, HourlyAllTracking> sitesfilesup;
  std::map<std::string, HourlyAllTracking> sitessizedown;
  std::map<std::string, HourlyAllTracking> sitesfilesdown;
  HourlyAllTracking sizeup;
  HourlyAllTracking filesup;
  HourlyAllTracking sizedown;
  HourlyAllTracking filesdown;
  bool freeslot;
public:
  Site();
  Site(const std::string &);
  Site(const Site &);
  std::map<std::string, Path>::const_iterator sectionsBegin() const;
  std::map<std::string, Path>::const_iterator sectionsEnd() const;
  std::map<std::string, int>::const_iterator avgspeedBegin() const;
  std::map<std::string, int>::const_iterator avgspeedEnd() const;
  unsigned int getMaxLogins() const;
  unsigned int getMaxUp() const;
  unsigned int getMaxDown() const;
  unsigned int getMaxDownPre() const;
  unsigned int getMaxDownComplete() const;
  unsigned int getMaxDownTransferJob() const;
  unsigned int getInternMaxLogins() const;
  unsigned int getInternMaxUp() const;
  unsigned int getInternMaxDown() const;
  unsigned int getInternMaxDownPre() const;
  unsigned int getInternMaxDownComplete() const;
  unsigned int getInternMaxDownTransferJob() const;
  bool getLeaveFreeSlot() const;
  const Path & getBasePath() const;
  bool unlimitedLogins() const;
  bool unlimitedUp() const;
  bool unlimitedDown() const;
  int getAverageSpeed(const std::string &) const;
  void setAverageSpeed(const std::string &, int);
  void pushTransferSpeed(const std::string &, int, unsigned long long int);
  std::pair<int, unsigned long long int> getAverageSpeedSamples(const std::string &) const;
  void resetAverageSpeedSamples(const std::string &);
  bool needsPRET() const;
  void setPRET(bool);
  bool forceBinaryMode() const;
  void setForceBinaryMode(bool);
  int getSSLTransferPolicy() const;
  int getListCommand() const;
  TLSMode getTLSMode() const;
  void setSSLTransferPolicy(int);
  void setListCommand(int);
  SitePriority getPriority() const;
  static std::string getPriorityText(SitePriority priority);
  void setPriority(SitePriority priority);
  bool hasBrokenPASV() const;
  void setBrokenPASV(bool);
  bool supportsSSCN() const;
  bool supportsCPSV() const;
  void setSupportsSSCN(bool);
  void setSupportsCPSV(bool);
  bool getDisabled() const;
  SiteAllowTransfer getAllowUpload() const;
  SiteAllowTransfer getAllowDownload() const;
  int getProxyType() const;
  std::string getProxy() const;
  unsigned int getMaxIdleTime() const;
  std::string getName() const;
  unsigned int sectionsSize() const;
  const Path getSectionPath(const std::string &) const;
  bool hasSection(const std::string &) const;
  std::string getAddress() const;
  std::string getPort() const;
  std::list<std::pair<std::string, std::string> > getAddresses() const;
  std::string getAddressesAsString() const;
  std::string getUser() const;
  std::string getPass() const;
  int getTransferSourcePolicy() const;
  int getTransferTargetPolicy() const;
  bool useXDUPE() const;
  SkipList & getSkipList();
  const std::map<std::string, Path> & getSections() const;
  void setName(const std::string &);
  void setAddresses(std::string);
  void setPrimaryAddress(const std::string &, const std::string &);
  void setBasePath(const std::string &);
  void setUser(const std::string &);
  void setPass(const std::string &);
  void setMaxLogins(unsigned int);
  void setMaxDn(unsigned int);
  void setMaxUp(unsigned int);
  void setMaxDnPre(unsigned int);
  void setMaxDnComplete(unsigned int);
  void setMaxDnTransferJob(unsigned int);
  void setLeaveFreeSlot(bool free);
  void setTLSMode(TLSMode mode);
  void setDisabled(bool);
  void setAllowUpload(SiteAllowTransfer);
  void setAllowDownload(SiteAllowTransfer);
  void setMaxIdleTime(unsigned int);
  void setProxyType(int);
  void setProxy(const std::string &);
  void clearSections();
  bool isAffiliated(const std::string &) const;
  void addAffil(const std::string &);
  void clearAffils();
  void setTransferSourcePolicy(int);
  void setTransferTargetPolicy(int);
  void setUseXDUPE(bool);
  void addAllowedSourceSite(const std::shared_ptr<Site> &);
  void addBlockedSourceSite(const std::shared_ptr<Site> &);
  void addExceptSourceSite(const std::shared_ptr<Site> &);
  void addAllowedTargetSite(const std::shared_ptr<Site> &);
  void addBlockedTargetSite(const std::shared_ptr<Site> &);
  void addExceptTargetSite(const std::shared_ptr<Site> &);
  void removeExceptSite(const std::shared_ptr<Site> &);
  void clearExceptSites();
  bool isAllowedTargetSite(const std::shared_ptr<Site> &) const;
  std::set<std::string>::const_iterator affilsBegin() const;
  std::set<std::string>::const_iterator affilsEnd() const;
  std::set<std::shared_ptr<Site> >::const_iterator exceptSourceSitesBegin() const;
  std::set<std::shared_ptr<Site> >::const_iterator exceptSourceSitesEnd() const;
  std::set<std::shared_ptr<Site> >::const_iterator exceptTargetSitesBegin() const;
  std::set<std::shared_ptr<Site> >::const_iterator exceptTargetSitesEnd() const;
  void addSection(const std::string &, const std::string &);
  void renameSection(const std::string & oldname, const std::string & newname);
  void removeSection(const std::string & name);
  std::list<std::string> getSectionsForPath(const Path &) const;
  std::list<std::string> getSectionsForPartialPath(const Path &) const;
  std::pair<Path, Path> splitPathInSectionAndSubpath(const Path &) const;
  void addTransferStatsFile(StatsDirection, const std::string &, unsigned long long int);
  void addTransferStatsFile(StatsDirection, unsigned long long int);
  void tickMinute();
  const HourlyAllTracking & getSizeUp() const;
  const HourlyAllTracking & getSizeDown() const;
  const HourlyAllTracking & getFilesUp() const;
  const HourlyAllTracking & getFilesDown() const;
  HourlyAllTracking & getSizeUp();
  HourlyAllTracking & getSizeDown();
  HourlyAllTracking & getFilesUp();
  HourlyAllTracking & getFilesDown();
  std::map<std::string, HourlyAllTracking>::const_iterator sizeUpBegin() const;
  std::map<std::string, HourlyAllTracking>::const_iterator filesUpBegin() const;
  std::map<std::string, HourlyAllTracking>::const_iterator sizeDownBegin() const;
  std::map<std::string, HourlyAllTracking>::const_iterator filesDownBegin() const;
  std::map<std::string, HourlyAllTracking>::const_iterator sizeUpEnd() const;
  std::map<std::string, HourlyAllTracking>::const_iterator filesUpEnd() const;
  std::map<std::string, HourlyAllTracking>::const_iterator sizeDownEnd() const;
  std::map<std::string, HourlyAllTracking>::const_iterator filesDownEnd() const;
  HourlyAllTracking & getSiteSizeUp(const std::string & site);
  HourlyAllTracking & getSiteSizeDown(const std::string & site);
  HourlyAllTracking & getSiteFilesUp(const std::string & site);
  HourlyAllTracking & getSiteFilesDown(const std::string & site);
  void setSkipList(const SkipList & skiplist);
  void setSections(const std::map<std::string, Path> & sections);
};
