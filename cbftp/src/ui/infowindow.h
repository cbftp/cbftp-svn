#pragma once

#include "fmtstring.h"

#include "uiwindow.h"

class InfoWindow : public UIWindow {
public:
  InfoWindow(Ui* ui, unsigned int row, unsigned int col);
  void redraw();
  void update();
  void setLabel(const FmtString& label);
  void setText(const FmtString& text);
  void setSplit(bool split);
private:
  FmtString label;
  FmtString text;
  bool split;
};
