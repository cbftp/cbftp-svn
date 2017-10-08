#pragma once

#include <vector>

#include "../../core/pointer.h"

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
  Pointer<Site> site;
  Pointer<SiteLogic> st;
};
