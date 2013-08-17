#pragma once

#include <string>
#include <map>
#include <list>

#define REPORT_LOGINS_IF_UNLIMITED 10

class Site {
  private:
    std::string name;
    std::string address;
    std::string port;
    std::string user;
    std::string pass;
    bool pret;
    bool sslconn;
    bool sslfxpforced;
    bool brokenpasv;
    unsigned int logins;
    unsigned int max_up;
    unsigned int max_dn;
    unsigned int max_idletime;
    bool allowupload;
    bool allowdownload;
    std::map<std::string, std::string> sections;
    std::map<std::string, int> avgspeed;
    std::map<std::string, bool> affils;
    std::string basepath;
  public:
    Site();
    Site(std::string);
    void copy(Site *);
    std::map<std::string, std::string>::iterator sectionsBegin();
    std::map<std::string, std::string>::iterator sectionsEnd();
    std::map<std::string, int>::iterator avgspeedBegin();
    std::map<std::string, int>::iterator avgspeedEnd();
    unsigned int getMaxLogins();
    unsigned int getMaxUp();
    unsigned int getMaxDown();
    unsigned int getInternMaxLogins();
    unsigned int getInternMaxUp();
    unsigned int getInternMaxDown();
    std::string getBasePath();
    bool unlimitedLogins();
    bool unlimitedUp();
    bool unlimitedDown();
    int getAverageSpeed(std::string);
    void setAverageSpeed(std::string, int);
    void pushTransferSpeed(std::string, int);
    bool needsPRET();
    void setPRET(bool);
    bool SSLFXPForced();
    bool SSL();
    void setSSLFXPForced(bool);
    bool hasBrokenPASV();
    void setBrokenPASV(bool);
    bool getAllowUpload();
    bool getAllowDownload();
    unsigned int getMaxIdleTime();
    std::string getName();
    std::string getSectionPath(std::string);
    bool hasSection(std::string);
    std::string getAddress();
    std::string getPort();
    std::string getUser();
    std::string getPass();
    void setName(std::string);
    void setAddress(std::string);
    void setBasePath(std::string);
    void setPort(std::string);
    void setUser(std::string);
    void setPass(std::string);
    void setMaxLogins(unsigned int);
    void setMaxDn(unsigned int);
    void setMaxUp(unsigned int);
    void setSSL(bool);
    void setAllowUpload(bool);
    void setAllowDownload(bool);
    void setMaxIdleTime(unsigned int);
    void clearSections();
    bool isAffiliated(std::string);
    void addAffil(std::string);
    void clearAffils();
    std::map<std::string, bool>::iterator affilsBegin();
    std::map<std::string, bool>::iterator affilsEnd();
    void addSection(std::string, std::string);
    std::list<std::string> getSectionsForPath(std::string);
};
