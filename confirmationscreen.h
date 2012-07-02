#pragma once

#include "uiwindow.h"
#include "uiwindowcommand.h"
#include "termint.h"

class ConfirmationScreen : public UIWindow {
public:
  ConfirmationScreen(WINDOW *, UIWindowCommand *, int, int);
  void redraw();
  void keyPressed(int);
  std::string getLegendText();
private:
  UIWindowCommand * windowcommand;
};
