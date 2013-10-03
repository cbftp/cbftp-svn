#pragma once

#include "../uiwindow.h"

class UICommunicator;

class ConfirmationScreen : public UIWindow {
public:
  ConfirmationScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void redraw();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
  UICommunicator * uicommunicator;
};
