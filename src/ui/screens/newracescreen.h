#pragma once

#include <ncurses.h>
#include <list>

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../engine.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../menuselectoption.h"
#include "../menuselectoptionelement.h"
#include "../termint.h"

extern GlobalContext * global;

class NewRaceScreen : public UIWindow {
public:
  NewRaceScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
private:
  Site * modsite;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  bool toggleall;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  UICommunicator * uicommunicator;
  std::string section;
  std::string release;
};
