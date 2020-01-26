#include "refreshgovernor.h"

#include "site.h"

RefreshGovernor::RefreshGovernor(const std::shared_ptr<Site>& site) : site(site),
                                 interval(0), timepassed(0)
{
  update();
}

RefreshGovernor::~RefreshGovernor() {

}

bool RefreshGovernor::refreshAllowed() const {
  return timepassed >= interval;
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
  switch (site->getPriority()) {
    case SitePriority::VERY_LOW:
      interval = 2000; // 0.5 times per second
      break;
    case SitePriority::LOW:
      interval = 500; // 2 times per second
      break;
    case SitePriority::NORMAL:
      interval = 200; // 5 times per second
      break;
    case SitePriority::HIGH:
      interval = 100; // 10 times per second
      break;
    case SitePriority::VERY_HIGH:
      interval = 50; // 20 times per second
      break;
  }
}
