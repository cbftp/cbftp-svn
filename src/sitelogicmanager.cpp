#include "sitelogicmanager.h"

#include "sitelogic.h"
#include "site.h"
#include "globalcontext.h"
#include "sitemanager.h"

SiteLogicManager::SiteLogicManager() {
}

const Pointer<SiteLogic> SiteLogicManager::getSiteLogic(const std::string & name) {
  std::vector<Pointer<SiteLogic> >::iterator it;
  for(it = sitelogics.begin(); it != sitelogics.end(); it++) {
    if ((*it)->getSite()->getName().compare(name) == 0) return *it;
  }
  if (!global->getSiteManager()->getSite(name)) {
    return Pointer<SiteLogic>();
  }
  Pointer<SiteLogic> x = makePointer<SiteLogic>(name);
  sitelogics.push_back(x);
  return x;
}

const Pointer<SiteLogic> SiteLogicManager::getSiteLogic(SiteLogic * sl) {
  std::vector<Pointer<SiteLogic> >::iterator it;
  for(it = sitelogics.begin(); it != sitelogics.end(); it++) {
    if ((*it).get() == sl) return *it;
  }
  return Pointer<SiteLogic>();
}

void SiteLogicManager::deleteSiteLogic(const std::string & name) {
  std::vector<Pointer<SiteLogic> >::iterator it;
  for(it = sitelogics.begin(); it != sitelogics.end(); it++) {
    if ((*it)->getSite()->getName().compare(name) == 0) {
      sitelogics.erase(it);
      return;
    }
  }
}
