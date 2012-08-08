#pragma once

#include "uiwindow.h"
#include "uicommunicator.h"
#include "menuselectoption.h"
#include "menuselectoptionelement.h"
#include "termint.h"

#define SHORTESTKEY 4

class NewKeyScreen : public UIWindow {
public:
  NewKeyScreen(WINDOW *, UICommunicator *, int, int);
  void update();
  void redraw();
  void keyPressed(int);
  std::string getLegendText();
private:
  std::string currentlegendtext;
  std::string defaultlegendtext;
  bool active;
  bool mismatch;
  bool tooshort;
  MenuSelectOptionElement * activeelement;
  MenuSelectOption mso;
  std::string operation;
  UICommunicator * uicommunicator;
};
