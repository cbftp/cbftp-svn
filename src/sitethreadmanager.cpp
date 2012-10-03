#include "sitethreadmanager.h"

SiteThreadManager::SiteThreadManager() {
}

SiteThread * SiteThreadManager::getSiteThread(std::string name) {
  std::vector<SiteThread *>::iterator it;
  for(it = sitethreads.begin(); it != sitethreads.end(); it++) {
    if ((*it)->getSite()->getName().compare(name) == 0) return *it;
  }
  SiteThread * x = new SiteThread(name);
  sitethreads.push_back(x);
  return x;
}

void SiteThreadManager::deleteSiteThread(std::string name) {
  std::vector<SiteThread *>::iterator it;
  for(it = sitethreads.begin(); it != sitethreads.end(); it++) {
    if ((*it)->getSite()->getName().compare(name) == 0) {
      delete *it;
      sitethreads.erase(it);
      return;
    }
  }
}
