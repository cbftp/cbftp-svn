#pragma once

#include "../uiwindow.h"
#include "../menuselectsite.h"
#include "../menuselectoption.h"

class FocusableArea;
class Ui;

class MainScreen : public UIWindow {
public:
  MainScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void update();
  void redraw();
  void command(std::string);
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  unsigned int currentviewspan;
  unsigned int sitestartrow;
  int currentraces;
  std::string msolegendtext;
  std::string msslegendtext;
  std::string gotolegendtext;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string deletesite;
  Ui * ui;
  MenuSelectSite mss;
  MenuSelectOption mso;
  bool gotomode;
};
