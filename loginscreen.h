#pragma once

#include "uiwindow.h"
#include "uiwindowcommand.h"
#include "textinputfield.h"
#include "termint.h"

class LoginScreen : public UIWindow {
public:
  LoginScreen(WINDOW *, UIWindowCommand *, int, int);
  void update();
  void redraw();
  void keyPressed(int);
private:
  UIWindowCommand * windowcommand;
  std::string passphrase;
  TextInputField passfield;
  int pass_row;
  int pass_col;
  bool attempt;
};
