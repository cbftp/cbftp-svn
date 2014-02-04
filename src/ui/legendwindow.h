#pragma once

#include <string>

#include "uiwindow.h"

class LegendWindow : public UIWindow {
public:
  LegendWindow(WINDOW *, int, int);
  void redraw();
  void update();
  void setText(std::string);
private:
  std::string text;
  int latestid;
  int latestcount;
  std::string latesttext;
  unsigned int currentpos;
};
