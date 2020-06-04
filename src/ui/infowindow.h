#pragma once

#include <string>

#include "uiwindow.h"

struct _win_st;
typedef struct _win_st WINDOW;

class InfoWindow : public UIWindow {
public:
  InfoWindow(Ui* ui, WINDOW* window, int row, int col);
  void redraw();
  void update();
  void setLabel(const std::basic_string<unsigned int>& label);
  void setText(const std::basic_string<unsigned int>& text);
  void setSplit(bool split);
private:
  std::basic_string<unsigned int> label;
  std::basic_string<unsigned int> text;
  bool split;
  WINDOW * window;
};
