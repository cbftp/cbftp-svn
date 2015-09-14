#pragma once

#include <string>

#include "menuselectoptionelement.h"
#include "textarrow.h"
#include "resizableelement.h"

class MenuSelectOptionTextArrow : public ResizableElement {
private:
  TextArrow arrow;
public:
  MenuSelectOptionTextArrow(std::string, int, int, std::string);
  std::string getContentText() const;
  void inputChar(int);
  bool activate();
  void deactivate();
  bool isActive() const;
  int getData() const;
  std::string getDataText() const;
  std::string getLegendText() const;
  void addOption(std::string, int);
  bool setOption(int);
  bool setOptionText(std::string);
  void clear();
  unsigned int wantedWidth() const;
};
