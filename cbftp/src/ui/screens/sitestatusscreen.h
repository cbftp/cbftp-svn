#pragma once

#include <vector>

#include "../uiwindow.h"

class Site;
class SiteLogic;

class SiteStatusScreen : public UIWindow {
public:
  SiteStatusScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  std::string sitename;
  std::vector<unsigned int> previousstatuslength;
  Site * site;
  SiteLogic * st;
};
