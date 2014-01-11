#pragma once

#include <list>

#include "../uiwindow.h"
#include "../menuselectoption.h"

class UICommunicator;
class MenuSelectOptionElement;
class Site;
class SiteLogic;

class NukeScreen : public UIWindow {
public:
  NukeScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  int nuke();
  std::string sitestr;
  SiteLogic * sitelogic;
  Site * site;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  std::string section;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  UICommunicator * uicommunicator;
  std::string release;
  std::string path;
};
