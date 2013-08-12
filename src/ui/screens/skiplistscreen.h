#pragma once

#include "../../globalcontext.h"
#include "../../skiplist.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../menuselectoptionelement.h"
#include "../menufilters.h"
#include "../menuselectoptioncontainer.h"
#include "../termint.h"

extern GlobalContext * global;

class SkipListScreen : public UIWindow {
public:
  SkipListScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  SkipList * skiplist;
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  MenuFilters mf;
  UICommunicator * uicommunicator;
};
