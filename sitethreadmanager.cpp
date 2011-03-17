#include "sitethreadmanager.h"

SiteThreadManager::SiteThreadManager() {
}

SiteThread * SiteThreadManager::getSiteThread(std::string name) {
  for (int i = 0; i < sitethreads.size(); i++) {
    if (sitethreads[i]->getSite()->getName().compare(name) == 0) return sitethreads[i];
  }
  SiteThread * x = new SiteThread(name);
  sitethreads.push_back(x);
  return x;
}
