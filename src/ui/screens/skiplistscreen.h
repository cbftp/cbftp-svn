#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../skiplist.h"

class MenuSelectOptionElement;
class Ui;
class FocusableArea;
class MenuSelectOptionTextField;
class MenuSelectAdjustableLine;

class SkipListScreen : public UIWindow {
public:
  SkipListScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void saveToTempSkipList();
  void addPatternLine(int, std::string, bool, bool, int, bool);
  void addPatternLine(int, std::string, bool, bool, int, bool, MenuSelectAdjustableLine *);
  SkipList * skiplist;
  std::string currentlegendtext;
  std::string baselegendtext;
  std::string tablelegendtext;
  bool active;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption base;
  MenuSelectOption table;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  SkipList testskiplist;
  MenuSelectOptionTextField * testpattern;
  MenuSelectOptionTextArrow * testtype;
};
