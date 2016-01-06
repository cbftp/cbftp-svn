#pragma once

#include <string>

#include "uiwindow.h"

struct _win_st;
typedef struct _win_st WINDOW;

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
  int staticcount;
  std::string latesttext;
  unsigned int offset;
  bool split;
  WINDOW * window;
};
