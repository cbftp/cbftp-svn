#pragma once

#include <string>

#include "menuselectoptionelement.h"

class MenuSelectOptionCheckBox : public MenuSelectOptionElement {
private:
  bool value;
public:
  MenuSelectOptionCheckBox(std::string, int, int, std::string, bool);
  std::string getContentText();
  bool activate();
  bool getData();
};
