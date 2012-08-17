#pragma once

#include "uiwindow.h"
#include "uicommunicator.h"
#include "termint.h"

class ConfirmationScreen : public UIWindow {
public:
  ConfirmationScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
private:
  UICommunicator * uicommunicator;
};
