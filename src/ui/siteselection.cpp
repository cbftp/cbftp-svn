#include "siteselection.h"

#include "../globalcontext.h"
#include "../sitemanager.h"
#include "../site.h"

void fillPreselectionList(const std::string& preselectstr, std::list<std::shared_ptr<Site>>* list) {
  std::list<std::string> preselectlist = util::trim(util::split(preselectstr, ","));
  for (std::list<std::string>::const_iterator it = preselectlist.begin(); it != preselectlist.end(); it++) {
    std::shared_ptr<Site> site = global->getSiteManager()->getSite(*it);
    list->push_back(site);
  }
}
