#pragma once

#include <map>
#include <string>
#include <utility>
#include <list>

#include "../uiwindow.h"

#include "../menuselectoption.h"

class SiteManager;
class MenuSelectOption;
class Site;

class SelectSitesScreen : public UIWindow {
public:
  SelectSitesScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string, std::string, Site *);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  SiteManager * sm;
  MenuSelectOption mso;
  std::string purpose;
  Site * skipsite;
  std::map<std::string, bool> preselected;
  std::list<std::pair<std::string, bool> > tempsites;
};
