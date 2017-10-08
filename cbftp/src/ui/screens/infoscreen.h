#pragma once

#include "../uiwindow.h"

class InfoScreen : public UIWindow {
public:
  InfoScreen(Ui *);
  void initialize(unsigned int, unsigned int);
  void redraw();
  void update();
  bool keyPressed(unsigned int);
  std::string getLegendText() const;
  std::string getInfoLabel() const;
};
