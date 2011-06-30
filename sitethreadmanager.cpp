#include "sitethreadmanager.h"

SiteThreadManager::SiteThreadManager() {
}

SiteThread * SiteThreadManager::getSiteThread(std::string name) {
  std::map<std::string, SiteThread *>::iterator it = sitethreads.find(name);
  if (it != sitethreads.end()) return it->second;
  SiteThread * x = new SiteThread(name);
  sitethreads[name] = x;
  return x;
}
