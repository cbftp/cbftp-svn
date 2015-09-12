#pragma once

#include <string>
#include <map>
#include <list>

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

#define SITE_RANK_MAX 100
#define SITE_RANK_USE_GLOBAL 0

class Site {
  private:
    std::string name;
    std::string address;
    std::string port;
    std::string user;
    std::string pass;
    bool pret;
    bool binary;
    bool sslconn;
    int listcommand;
    int ssltransfer;
    bool brokenpasv;
    bool cpsvsupported;
    unsigned int logins;
    unsigned int max_up;
    unsigned int max_dn;
    unsigned int max_idletime;
    bool allowupload;
    bool allowdownload;
    std::map<std::string, std::string> sections;
    std::map<std::string, int> avgspeed;
    std::map<std::string, bool> affils;
    std::map<std::string, bool> bannedgroups;
    std::string basepath;
    int proxytype;
    std::string proxyname;
    int rank;      // Only pair sites dst->rank >= (dst->rank - dst->rank_tol)
    int ranktolerance; 
  public:
    Site();
    Site(std::string);
    std::map<std::string, std::string>::const_iterator sectionsBegin() const;
    std::map<std::string, std::string>::const_iterator sectionsEnd() const;
    std::map<std::string, int>::const_iterator avgspeedBegin() const;
    std::map<std::string, int>::const_iterator avgspeedEnd() const;
    unsigned int getMaxLogins() const;
    unsigned int getMaxUp() const;
    unsigned int getMaxDown() const;
    unsigned int getInternMaxLogins() const;
    unsigned int getInternMaxUp() const;
    unsigned int getInternMaxDown() const;
    std::string getBasePath() const;
    bool unlimitedLogins() const;
    bool unlimitedUp() const;
    bool unlimitedDown() const;
    int getAverageSpeed(std::string) const;
    void setAverageSpeed(std::string, int);
    void pushTransferSpeed(std::string, int);
    bool needsPRET() const;
    void setPRET(bool);
    bool forceBinaryMode() const;
    void setForceBinaryMode(bool);
    int getSSLTransferPolicy() const;
    int getListCommand() const;
    bool SSL() const;
    void setSSLTransferPolicy(int);
    void setListCommand(int);
    bool hasBrokenPASV() const;
    void setBrokenPASV(bool);
    bool supportsCPSV() const;
    void setSupportsCPSV(bool);
    bool getAllowUpload() const;
    bool getAllowDownload() const;
    int getProxyType() const;
    std::string getProxy() const;
    unsigned int getMaxIdleTime() const;
    std::string getName() const;
    std::string getSectionPath(std::string) const;
    bool hasSection(std::string) const;
    std::string getAddress() const;
    std::string getPort() const;
    std::string getUser() const;
    std::string getPass() const;
    int getRank() const;
    int getRankTolerance() const;
    void setName(std::string);
    void setAddress(std::string);
    void setBasePath(std::string);
    void setPort(std::string);
    void setUser(std::string);
    void setPass(std::string);
    void setRank(int);
    void setRankTolerance(int);
    void setMaxLogins(unsigned int);
    void setMaxDn(unsigned int);
    void setMaxUp(unsigned int);
    void setSSL(bool);
    void setAllowUpload(bool);
    void setAllowDownload(bool);
    void setMaxIdleTime(unsigned int);
    void setProxyType(int);
    void setProxy(std::string);
    void clearSections();
    bool isAffiliated(const std::string &) const;
    void addAffil(const std::string &);
    void clearAffils();
    bool isBannedGroup(const std::string &) const;
    void addBannedGroup(const std::string &);
    void clearBannedGroups();
    std::map<std::string, bool>::const_iterator affilsBegin() const;
    std::map<std::string, bool>::const_iterator affilsEnd() const;
    std::map<std::string, bool>::const_iterator bannedGroupsBegin() const;
    std::map<std::string, bool>::const_iterator bannedGroupsEnd() const;
    void addSection(std::string, std::string);
    std::list<std::string> getSectionsForPath(std::string) const;
    std::list<std::string> getSectionsForPartialPath(std::string) const;
};
