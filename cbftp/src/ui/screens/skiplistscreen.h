#pragma once

#include "../uiwindow.h"
#include "../menuselectoption.h"

#include "../../core/pointer.h"
#include "../../skiplist.h"

class SkipList;
class Ui;
class FocusableArea;
class MenuSelectAdjustableLine;
class MenuSelectOptionTextField;
class MenuSelectOptionTextArrow;
class MenuSelectOptionElement;

class SkipListScreen : public UIWindow {
public:
  SkipListScreen(Ui *);
  ~SkipListScreen();
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  void saveToTempSkipList();
  void addPatternLine(int, std::string, bool, bool, int, bool);
  void addPatternLine(int, std::string, bool, bool, int, bool, Pointer<MenuSelectAdjustableLine>);
  SkipList * skiplist;
  std::string currentlegendtext;
  std::string baselegendtext;
  std::string tablelegendtext;
  bool active;
  Pointer<MenuSelectOptionElement> activeelement;
  MenuSelectOption base;
  MenuSelectOption table;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  SkipList testskiplist;
  Pointer<MenuSelectOptionTextField> testpattern;
  Pointer<MenuSelectOptionTextArrow> testtype;
  unsigned int currentviewspan;
};
