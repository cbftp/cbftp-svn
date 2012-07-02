#pragma once

#include "uiwindow.h"
#include "menuselectsite.h"
#include "uiwindowcommand.h"
#include "site.h"
#include "termint.h"

class MainScreen : public UIWindow {
public:
  MainScreen(WINDOW *, UIWindowCommand *, int, int);
  void update();
  void redraw();
  void keyPressed(int);
  std::string getLegendText();
private:
  std::string deletesite;
  UIWindowCommand * windowcommand;
  MenuSelectSite mss;
};
