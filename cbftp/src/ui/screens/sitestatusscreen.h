#pragma once

#include <memory>
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
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  std::string sitename;
  std::shared_ptr<Site> site;
  std::shared_ptr<SiteLogic> st;
};
