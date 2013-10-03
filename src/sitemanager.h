#pragma once

#include <string>
#include <vector>

#define DEFAULTUSERNAME "anonymous"
#define DEFAULTPASSWORD "anonymous"
#define DEFAULTMAXLOGINS 3
#define DEFAULTMAXUP 0
#define DEFAULTMAXDOWN 2
#define DEFAULTMAXIDLETIME 60
#define DEFAULTSSL true
#define DEFAULTSSLFXPFORCED false

class Site;

class SiteManager {
  private:
    std::vector<Site *> sites;
    std::string defaultusername;
    std::string defaultpassword;
    unsigned int defaultmaxlogins;
    unsigned int defaultmaxup;
    unsigned int defaultmaxdown;
    unsigned int defaultmaxidletime;
    bool defaultsslfxpforced;
    bool defaultsslconn;
  public:
    SiteManager();
    void readConfiguration();
    int getNumSites();
    void addSite(Site *);
    Site * getSite(std::string);
    void deleteSite(std::string);
    std::vector<Site *>::iterator getSitesIteratorBegin();
    std::vector<Site *>::iterator getSitesIteratorEnd();
    std::string getDefaultUserName();
    void setDefaultUserName(std::string);
    std::string getDefaultPassword();
    void setDefaultPassword(std::string);
    unsigned int getDefaultMaxLogins();
    void setDefaultMaxLogins(unsigned int);
    unsigned int getDefaultMaxUp();
    void setDefaultMaxUp(unsigned int);
    unsigned int getDefaultMaxDown();
    void setDefaultMaxDown(unsigned int);
    unsigned int getDefaultMaxIdleTime();
    void setDefaultMaxIdleTime(unsigned int);
    bool getDefaultSSL();
    void setDefaultSSL(bool);
    bool getDefaultSSLFXPForced();
    void setDefaultSSLFXPForced(bool);
    void writeState();
    void sortSites();
};

bool siteNameComparator(Site *, Site *);
