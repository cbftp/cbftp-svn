#pragma once

#include <string>
#include <ncurses.h>

#include "uiwindow.h"

class InfoWindow : public UIWindow {
public:
  InfoWindow(Ui *, WINDOW *, int, int);
  void redraw();
  void update();
  void setLabel(std::string);
  void setText(std::string);
private:
  std::string label;
  std::string text;
  WINDOW * window;
};
