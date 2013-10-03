#pragma once

#include "../uiwindow.h"
#include "../menuselectsite.h"
#include "../menuselectoption.h"

class FocusableArea;
class UICommunicator;

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
  int currentraces;
  std::string msolegendtext;
  std::string msslegendtext;
  FocusableArea * focusedarea;
  FocusableArea * defocusedarea;
  std::string deletesite;
  UICommunicator * uicommunicator;
  MenuSelectSite mss;
  MenuSelectOption mso;
};
