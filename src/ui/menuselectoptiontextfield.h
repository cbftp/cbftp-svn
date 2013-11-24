#pragma once

#include <string>

#include "textinputfield.h"
#include "menuselectoptionelement.h"

class MenuSelectOptionTextField : public MenuSelectOptionElement {
private:
  TextInputField textfield;
public:
  MenuSelectOptionTextField();
  MenuSelectOptionTextField(std::string, int, int, std::string, std::string, int, int, bool);
  std::string getContentText();
  bool activate();
  void inputChar(int);
  int cursorPosition();
  std::string getData();
  void clear();
  void setText(std::string);
  std::string getLegendText();
};
