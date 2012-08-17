#pragma once

#include "uiwindow.h"
#include "termint.h"

class LegendWindow : public UIWindow {
public:
  LegendWindow(WINDOW *, int, int);
  void redraw();
  void update();
  void setText(std::string);
private:
  std::string text;
  unsigned int currentpos;
};
