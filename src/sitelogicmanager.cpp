#include "sitelogicmanager.h"

#include "sitelogic.h"
#include "site.h"
#include "globalcontext.h"
#include "sitemanager.h"

SiteLogicManager::SiteLogicManager() {
}

SiteLogic * SiteLogicManager::getSiteLogic(std::string name) {
  std::vector<SiteLogic *>::iterator it;
  for(it = sitelogics.begin(); it != sitelogics.end(); it++) {
    if ((*it)->getSite()->getName().compare(name) == 0) return *it;
  }
  if (global->getSiteManager()->getSite(name) == NULL) {
    return NULL;
  }
  SiteLogic * x = new SiteLogic(name);
  sitelogics.push_back(x);
  return x;
}

void SiteLogicManager::deleteSiteLogic(std::string name) {
  std::vector<SiteLogic *>::iterator it;
  for(it = sitelogics.begin(); it != sitelogics.end(); it++) {
    if ((*it)->getSite()->getName().compare(name) == 0) {
      delete *it;
      sitelogics.erase(it);
      return;
    }
  }
}
