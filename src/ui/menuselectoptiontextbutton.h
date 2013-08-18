#pragma once

#include <string>

#include "menuselectoptionelement.h"

class MenuSelectOptionTextButton : public MenuSelectOptionElement {
private:
  std::string text;
public:
  MenuSelectOptionTextButton(std::string, int, int, std::string);
  MenuSelectOptionTextButton(std::string, int, int, std::string, bool);
  std::string getContentText();
  bool isActivated();
  bool activate();
};
