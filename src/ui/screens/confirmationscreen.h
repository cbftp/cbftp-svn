#pragma once

#include "../uiwindow.h"

class ConfirmationScreen : public UIWindow {
public:
  ConfirmationScreen(Ui *);
  void initialize(unsigned int, unsigned int, std::string);
  void redraw();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
private:
  std::string message;
};
