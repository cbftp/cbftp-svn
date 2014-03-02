#pragma once

#include "../uiwindow.h"

#include "../menuselectoption.h"

class UICommunicator;
class SiteManager;
class MenuSelectOption;

class SelectSitesScreen : public UIWindow {
public:
  SelectSitesScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  UICommunicator * uicommunicator;
  SiteManager * sm;
  MenuSelectOption mso;
  std::string purpose;
};
