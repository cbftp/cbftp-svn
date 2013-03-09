#pragma once

#include "uiwindow.h"
#include "termint.h"

class InfoWindow : public UIWindow {
public:
  InfoWindow(WINDOW *, int, int);
  void redraw();
  void update();
  void setLabel(std::string);
  void setText(std::string);
private:
  std::string label;
  std::string text;
};
