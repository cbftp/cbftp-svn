#pragma once

#include "../../site.h"

#include "../uiwindow.h"
#include "../menuselectsite.h"
#include "../uicommunicator.h"
#include "../termint.h"

class MainScreen : public UIWindow {
public:
  MainScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
private:
  std::string deletesite;
  UICommunicator * uicommunicator;
  MenuSelectSite mss;
};
