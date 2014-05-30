#pragma once

#include "../uiwindow.h"
#include "../menufilters.h"

class MenuSelectOptionElement;
class SkipList;
class Ui;

class SkipListScreen : public UIWindow {
public:
  SkipListScreen(Ui *);
  void initialize(unsigned int, unsigned int);
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
};
