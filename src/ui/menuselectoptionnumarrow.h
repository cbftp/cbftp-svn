#pragma once

#include <string>

#include "menuselectoptionelement.h"
#include "numinputarrow.h"

class MenuSelectOptionNumArrow : public MenuSelectOptionElement {
private:
  NumInputArrow arrow;
public:
  MenuSelectOptionNumArrow(std::string, int, int, std::string, int, int, int);
  std::string getContentText();
  void inputChar(int);
  bool activate();
  void deactivate();
  int getData();
  std::string getLegendText();
};
