#pragma once

#include "../textinputfield.h"
#include "../uiwindow.h"

class UICommunicator;

class LoginScreen : public UIWindow {
public:
  LoginScreen(WINDOW *, UICommunicator *, unsigned int, unsigned int);
  void update();
  void redraw();
  void keyPressed(unsigned int);
private:
  UICommunicator * uicommunicator;
  std::string passphrase;
  TextInputField passfield;
  int pass_row;
  int pass_col;
  bool attempt;
};
