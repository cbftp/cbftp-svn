#pragma once

#include "../../site.h"
#include "../../race.h"
#include "../../engine.h"
#include "../../sitemanager.h"
#include "../../site.h"

#include "../uiwindow.h"
#include "../menuselectsite.h"
#include "../menuselectoption.h"
#include "../menuselectoptioncheckbox.h"
#include "../uicommunicator.h"
#include "../termint.h"
#include "../focusablearea.h"

class MainScreen : public UIWindow {
public:
  MainScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  unsigned int currentviewspan;
  unsigned int sitestartrow;
  std::string msolegendtext;
  std::string msslegendtext;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string deletesite;
  UICommunicator * uicommunicator;
  MenuSelectSite mss;
  MenuSelectOption mso;
};
