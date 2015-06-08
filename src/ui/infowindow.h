#pragma once

#include <string>

#include "uiwindow.h"

struct _win_st;
typedef struct _win_st WINDOW;

class InfoWindow : public UIWindow {
public:
  InfoWindow(Ui *, WINDOW *, int, int);
  void redraw();
  void update();
  void setLabel(std::string);
  void setText(std::string);
  void setSplit(bool);
private:
  std::string label;
  std::string text;
  bool split;
  WINDOW * window;
};
