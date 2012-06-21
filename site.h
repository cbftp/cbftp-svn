#pragma once

#include <string>
#include <map>

class Site {
  private:
    std::string name;
    std::string address;
    std::string port;
    std::string user;
    std::string pass;
    bool pret;
    bool brokenpasv;
    int logins;
    int max_up;
    int max_dn;
    std::map<std::string, std::string> sections;
    std::map<std::string, int> avgspeed;
  public:
    Site();
    Site(std::string);
    void copy(Site *);
    std::map<std::string, std::string>::iterator sectionsBegin();
    std::map<std::string, std::string>::iterator sectionsEnd();
    std::map<std::string, int>::iterator avgspeedBegin();
    std::map<std::string, int>::iterator avgspeedEnd();
    int getMaxLogins();
    int getMaxUp();
    int getMaxDown();
    int getAverageSpeed(std::string);
    void setAverageSpeed(std::string, int);
    void pushTransferSpeed(std::string, int);
    bool needsPRET();
    void setPRET(bool);
    bool hasBrokenPASV();
    void setBrokenPASV(bool);
    std::string getName();
    std::string getSectionPath(std::string);
    std::string getAddress();
    std::string getPort();
    std::string getUser();
    std::string getPass();
    void setName(std::string);
    void setAddress(std::string);
    void setPort(std::string);
    void setUser(std::string);
    void setPass(std::string);
    void setMaxLogins(int);
    void setMaxDn(int);
    void setMaxUp(int);
    void addSection(std::string, std::string);
};
