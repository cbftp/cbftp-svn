#pragma once

#include "textinputfield.h"
#include "menuselectoptionelement.h"
#include <string>
#include <ncurses.h>

class MenuSelectOptionTextField : public MenuSelectOptionElement {
private:
  TextInputField textfield;
public:
  MenuSelectOptionTextField(std::string, int, int, std::string, std::string, int, int, bool);
  std::string getContentText();
  void inputChar(int);
  int cursorPosition();
  std::string getData();
};
