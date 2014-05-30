#pragma once

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
  std::string getLegendText();
  std::string getInfoLabel();
private:
  SiteManager * sm;
  MenuSelectOption mso;
  std::string purpose;
};
