#pragma once

#include "uiwindow.h"
#include "uiwindowcommand.h"

class ConfirmationScreen : public UIWindow {
public:
  ConfirmationScreen(WINDOW *, UIWindowCommand *, int, int);
  void redraw();
  void keyPressed(int);
private:
  UIWindowCommand * windowcommand;
};
