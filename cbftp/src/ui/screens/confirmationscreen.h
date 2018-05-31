#pragma once

#include "../uiwindow.h"

class ConfirmationScreen : public UIWindow {
public:
  ConfirmationScreen(Ui *);
  void initialize(unsigned int row, unsigned int col, const std::string & message, bool strong);
  void redraw();
  bool keyPressed(unsigned int ch);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  std::string message;
  bool strong;
  int strongconfirmstep;
};
