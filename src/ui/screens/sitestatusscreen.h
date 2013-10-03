#pragma once

#include <vector>

#include "../uiwindow.h"

class Site;
class SiteLogic;
class UICommunicator;

class SiteStatusScreen : public UIWindow {
public:
  SiteStatusScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  std::string sitename;
  std::vector<unsigned int> previousstatuslength;
  Site * site;
  SiteLogic * st;
  UICommunicator * uicommunicator;
};
