#pragma once

#include "uiwindow.h"
#include "uicommunicator.h"
#include "textinputfield.h"
#include "termint.h"

class LoginScreen : public UIWindow {
public:
  LoginScreen(WINDOW *, UICommunicator *, int, int);
  void update();
  void redraw();
  void keyPressed(int);
private:
  UICommunicator * uicommunicator;
  std::string passphrase;
  TextInputField passfield;
  int pass_row;
  int pass_col;
  bool attempt;
};
