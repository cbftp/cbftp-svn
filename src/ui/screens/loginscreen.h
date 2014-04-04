#pragma once

#include <vector>

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
  std::string drawword;
  void randomizeDrawLocation();
  UICommunicator * uicommunicator;
  std::vector<std::vector<int> > background;
  std::string passphrase;
  TextInputField passfield;
  int pass_row;
  int pass_col;
  int drawx;
  int drawy;
  bool attempt;
};
