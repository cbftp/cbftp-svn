#pragma once

#include <map>
#include <string>
#include <utility>
#include <list>
#include <set>

#include "../uiwindow.h"

#include "../menuselectoption.h"

class SiteManager;
class MenuSelectOption;
class Site;

class SelectSitesScreen : public UIWindow {
public:
  SelectSitesScreen(Ui *);
  void initializeSelect(unsigned int, unsigned int, const std::string &, std::list<Pointer<Site> >, std::list<Pointer<Site> >);
  void initializeExclude(unsigned int, unsigned int, const std::string &, std::list<Pointer<Site> >, std::list<Pointer<Site> >);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  SiteManager * sm;
  MenuSelectOption mso;
  std::string purpose;
  std::list<std::pair<std::string, bool> > tempsites;
  bool togglestate;
};
