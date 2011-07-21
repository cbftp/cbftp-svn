#include "site.h"

Site::Site(std::string name) {
  this->name = name;
  address = "ftp.sunet.se";
  port = "21";
  user = "anonymous";
  pass = "anonymous";
  logins = 0;
  max_up = 0;
  max_dn = 0;
  pret = false;
  brokenpasv = false;
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
  this->brokenpasv = site->brokenpasv;
  this->sections = site->sections;
  this->avgspeed = site->avgspeed;
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

int Site::getMaxLogins() {
  return logins;
}

int Site::getMaxUp() {
  return max_up;
}

int Site::getMaxDown() {
  return max_dn;
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
  avgspeed[target] = (speed / 5) + (oldspeed * 0.8);
}

bool Site::needsPRET() {
  return pret;
}

void Site::setPRET(bool val) {
  pret = val;
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

void Site::setMaxLogins(int num) {
  logins = num;
}

void Site::setMaxDn(int num) {
  max_dn = num > logins ? logins : num;
}

void Site::setMaxUp(int num) {
  max_up = num > logins ? logins : num;
}

void Site::addSection(std::string name, std::string path) {
  sections[name] = path;
}
