#pragma once

#include "../uiwindow.h"
#include "../menufilters.h"

class UICommunicator;
class MenuSelectOptionElement;
class SkipList;

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
