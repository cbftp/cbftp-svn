#pragma once

#include <memory>

class Site;

class RefreshGovernor {
public:
  RefreshGovernor(const std::shared_ptr<Site>& site);
  ~RefreshGovernor();
  bool refreshAllowed() const; // is it time to perform a refresh?
  void timePassed(int timepassed); // called continously by the sitelogic ticker
  void useRefresh(); // call when a refresh has been "taken" by a connection
  void update(); // call when site settings have changed
private:
  std::shared_ptr<Site> site;
  int interval;
  int timepassed;
};
