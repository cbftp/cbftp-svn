#pragma once

#include "../uiwindow.h"

class InfoScreen : public UIWindow {
public:
  InfoScreen(Ui* ui);
  void initialize(unsigned int row, unsigned int col);
  void redraw();
  bool keyPressed(unsigned int ch);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
};
