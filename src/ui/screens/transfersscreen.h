#pragma once

#include "../uiwindow.h"

class TransfersScreen : public UIWindow {
public:
  TransfersScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  void keyPressed(unsigned int);
  std::string getLegendText();
  std::string getInfoLabel();
private:
};
