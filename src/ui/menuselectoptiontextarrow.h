#pragma once

#include <string>

#include "menuselectoptionelement.h"
#include "textarrow.h"

class MenuSelectOptionTextArrow : public MenuSelectOptionElement {
private:
  TextArrow arrow;
public:
  MenuSelectOptionTextArrow(std::string, int, int, std::string);
  std::string getContentText();
  void inputChar(int);
  bool activate();
  void deactivate();
  bool isActive();
  int getData();
  std::string getDataText();
  std::string getLegendText();
  void addOption(std::string, int);
  bool setOption(int);
  bool setOptionText(std::string);
  void clear();
};
