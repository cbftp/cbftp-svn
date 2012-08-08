#pragma once

#include "uiwindow.h"
#include "uicommunicator.h"
#include "termint.h"

class ConfirmationScreen : public UIWindow {
public:
  ConfirmationScreen(WINDOW *, UICommunicator *, int, int);
  void redraw();
  void keyPressed(int);
  std::string getLegendText();
private:
  UICommunicator * uicommunicator;
};
