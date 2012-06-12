#include "menuselectoption.h"

MenuSelectOption::MenuSelectOption() {
  pointer = 0;
  lastpointer = 0;
}


bool MenuSelectOption::goNext() {
  if (pointer == size() - 1) {
    return false;
  }
  lastpointer = pointer;
  pointer++;
  return true;
}

bool MenuSelectOption::goPrev() {
  if (pointer == 0) {
    return false;
  }
  lastpointer = pointer;
  pointer--;
  return true;
}

void MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret) {
  options.push_back(new MenuSelectOptionTextField(identifier, row, col, label, starttext, 32, 32, secret));
}

void MenuSelectOption::addIntArrow(int row, int col, std::string identifier, std::string label, int startval, int min, int max) {
  options.push_back(new MenuSelectOptionNumArrow(identifier, row, col, label, startval, min, max));
}

void MenuSelectOption::addCheckBox(int row, int col, std::string identifier, std::string label, bool startval) {
  options.push_back(new MenuSelectOptionCheckBox(identifier, row, col, label, startval));
}

MenuSelectOptionElement * MenuSelectOption::getElement(int i) {
  if (i < 0 || i > size()) {
    return NULL;
  }
  return options[i];
}

int MenuSelectOption::getLastSelectionPointer() {
  return lastpointer;
}

int MenuSelectOption::getSelectionPointer() {
  return pointer;
}
void MenuSelectOption::clear() {
  std::vector<MenuSelectOptionElement *>::iterator it;
  for (it = options.begin(); it != options.end(); it++) {
    delete *it;
  }
  options.clear();
}

int MenuSelectOption::size() {
  return options.size();
}
