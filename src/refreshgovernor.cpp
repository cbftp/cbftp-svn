#include "refreshgovernor.h"

#include "site.h"
#include "globalcontext.h"
#include "loadmonitor.h"

namespace {

struct RefreshItem {
  RefreshItem(float timespersecond, bool immediate) : interval(1000 / timespersecond), immediate(immediate) {

  }
  unsigned int interval;
  bool immediate;
};

std::map<unsigned int, std::map<SitePriority, RefreshItem>> populateRefreshRateMap() {
  std::map<unsigned int, std::map<SitePriority, RefreshItem>> rates;
  rates[1].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(0.5, false)));
  rates[2].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(0.5, false)));
  rates[3].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(1, false)));
  rates[4].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(1, false)));
  rates[5].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(2, false)));
  rates[6].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(2, false)));
  rates[7].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(3, false)));
  rates[8].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(4, false)));
  rates[9].insert(std::make_pair(SitePriority::VERY_LOW, RefreshItem(5, true)));
  rates[1].insert(std::make_pair(SitePriority::LOW, RefreshItem(0.5, false)));
  rates[2].insert(std::make_pair(SitePriority::LOW, RefreshItem(1, false)));
  rates[3].insert(std::make_pair(SitePriority::LOW, RefreshItem(1, false)));
  rates[4].insert(std::make_pair(SitePriority::LOW, RefreshItem(2, false)));
  rates[5].insert(std::make_pair(SitePriority::LOW, RefreshItem(3, false)));
  rates[6].insert(std::make_pair(SitePriority::LOW, RefreshItem(4, false)));
  rates[7].insert(std::make_pair(SitePriority::LOW, RefreshItem(6, false)));
  rates[8].insert(std::make_pair(SitePriority::LOW, RefreshItem(8, true)));
  rates[9].insert(std::make_pair(SitePriority::LOW, RefreshItem(10, true)));
  rates[1].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(1, false)));
  rates[2].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(2, false)));
  rates[3].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(3, false)));
  rates[4].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(4, false)));
  rates[5].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(5, false)));
  rates[6].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(8, false)));
  rates[7].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(10, true)));
  rates[8].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(15, true)));
  rates[9].insert(std::make_pair(SitePriority::NORMAL, RefreshItem(20, true)));
  rates[1].insert(std::make_pair(SitePriority::HIGH, RefreshItem(2, false)));
  rates[2].insert(std::make_pair(SitePriority::HIGH, RefreshItem(3, false)));
  rates[3].insert(std::make_pair(SitePriority::HIGH, RefreshItem(4, false)));
  rates[4].insert(std::make_pair(SitePriority::HIGH, RefreshItem(5, false)));
  rates[5].insert(std::make_pair(SitePriority::HIGH, RefreshItem(8, false)));
  rates[6].insert(std::make_pair(SitePriority::HIGH, RefreshItem(12, true)));
  rates[7].insert(std::make_pair(SitePriority::HIGH, RefreshItem(15, true)));
  rates[8].insert(std::make_pair(SitePriority::HIGH, RefreshItem(18, true)));
  rates[9].insert(std::make_pair(SitePriority::HIGH, RefreshItem(20, true)));
  rates[1].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(2, false)));
  rates[2].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(5, false)));
  rates[3].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(8, false)));
  rates[4].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(10, true)));
  rates[5].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(15, true)));
  rates[6].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(18, true)));
  rates[7].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(20, true)));
  rates[8].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(20, true)));
  rates[9].insert(std::make_pair(SitePriority::VERY_HIGH, RefreshItem(20, true)));
  return rates;
}

std::map<unsigned int, std::map<SitePriority, RefreshItem>> rates = populateRefreshRateMap();

}

RefreshGovernor::RefreshGovernor(const std::shared_ptr<Site>& site) : site(site),
                                 interval(0), timepassed(0), immediaterefreshallowed(true)
{
  update();
  global->getLoadMonitor()->addListener(this);
}

RefreshGovernor::~RefreshGovernor() {
  global->getLoadMonitor()->removeListener(this);
}

bool RefreshGovernor::refreshAllowed() const {
  return timepassed >= interval;
}

bool RefreshGovernor::immediateRefreshAllowed() const {
  return immediaterefreshallowed;
}

void RefreshGovernor::timePassed(int timepassed) {
  this->timepassed += timepassed;
}

void RefreshGovernor::useRefresh() {
  if (timepassed >= interval) {
    timepassed %= interval;
  }
  else {
    timepassed = 0;
  }
}

void RefreshGovernor::update() {
  recommendedPerformanceLevelChanged(global->getLoadMonitor()->getCurrentRecommendedPerformanceLevel());
}

void RefreshGovernor::recommendedPerformanceLevelChanged(int newlevel) {
  if (site->getListCommand() == SITE_LIST_LIST) {
    interval = 1000;
    immediaterefreshallowed = false;
    return;
  }
  switch (site->getRefreshRate()) {
    case RefreshRate::VERY_LOW:
      interval = 1000;
      immediaterefreshallowed = false;
      break;
    case RefreshRate::LOW:
      interval = 500;
      immediaterefreshallowed = false;
      break;
    case RefreshRate::AVERAGE:
      interval = 166;
      immediaterefreshallowed = true;
      break;
    case RefreshRate::HIGH:
      interval = 100;
      immediaterefreshallowed = true;
      break;
    case RefreshRate::VERY_HIGH:
      interval = 50;
      immediaterefreshallowed = true;
      break;
    case RefreshRate::DYNAMIC:
    {
      const RefreshItem& item = rates.at(newlevel).at(site->getPriority());
      interval = item.interval;
      immediaterefreshallowed = item.immediate;
      break;
    }
  }
}
