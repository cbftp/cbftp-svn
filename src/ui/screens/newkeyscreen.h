#pragma once

#include "../../globalcontext.h"

#include "../uiwindow.h"
#include "../uicommunicator.h"
#include "../menuselectoption.h"
#include "../menuselectoptionelement.h"
#include "../termint.h"

#define SHORTESTKEY 4

extern GlobalContext * global;

class NewKeyScreen : public UIWindow {
public:
  NewKeyScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
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
