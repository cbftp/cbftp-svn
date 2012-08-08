#pragma once

#include "uiwindow.h"
#include "menuselectsite.h"
#include "uicommunicator.h"
#include "site.h"
#include "termint.h"

class MainScreen : public UIWindow {
public:
  MainScreen(WINDOW *, UICommunicator *, int, int);
  void update();
  void redraw();
  void keyPressed(int);
  std::string getLegendText();
private:
  std::string deletesite;
  UICommunicator * uicommunicator;
  MenuSelectSite mss;
};
