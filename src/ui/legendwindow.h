#pragma once

#include <string>
#include <ncurses.h>

#include "uiwindow.h"

class LegendWindow : public UIWindow {
public:
  LegendWindow(Ui *, WINDOW *, int, int);
  void redraw();
  void update();
  void setText(std::string);
  void setSplit(bool);
private:
  std::string text;
  int latestid;
  int latestcount;
  std::string latesttext;
  unsigned int currentpos;
  bool split;
  WINDOW * window;
};
