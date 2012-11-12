#include "menuselectoption.h"

MenuSelectOption::MenuSelectOption() {
  pointer = 0;
  lastpointer = 0;
}


bool MenuSelectOption::goDown() {
  if (pointer == size() - 1) {
    if (leavedown) {
      lastpointer = pointer;
      focus = false;
      return true;
    }
    return false;
  }
  lastpointer = pointer;
  pointer++;
  return true;
}

bool MenuSelectOption::goUp() {
  if (pointer == 0) {
    if (leaveup) {
      lastpointer = pointer;
      focus = false;
      return true;
    }
    return false;
  }
  lastpointer = pointer;
  pointer--;
  return true;
}

bool MenuSelectOption::goRight() {
  if (leaveright) {
    focus = false;
    return true;
  }
  return false;
}

bool MenuSelectOption::goLeft() {
  if (leaveleft) {
    focus = false;
    return true;
  }
  return false;
}

void MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret) {
  addStringField(row, col, identifier, label, starttext, secret, 32);
}

void MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret, int maxlen) {
  options.push_back(new MenuSelectOptionTextField(identifier, row, col, label, starttext, maxlen, maxlen, secret));
}

void MenuSelectOption::addIntArrow(int row, int col, std::string identifier, std::string label, int startval, int min, int max) {
  options.push_back(new MenuSelectOptionNumArrow(identifier, row, col, label, startval, min, max));
}

void MenuSelectOption::addCheckBox(int row, int col, std::string identifier, std::string label, bool startval) {
  options.push_back(new MenuSelectOptionCheckBox(identifier, row, col, label, startval));
}

MenuSelectOptionElement * MenuSelectOption::getElement(unsigned int i) {
  if (i < 0 || i >= size()) {
    return NULL;
  }
  return options[i];
}

unsigned int MenuSelectOption::getLastSelectionPointer() {
  return lastpointer;
}

unsigned int MenuSelectOption::getSelectionPointer() {
  return pointer;
}

bool MenuSelectOption::activateSelected() {
  return getElement(pointer)->activate();
}

void MenuSelectOption::clear() {
  std::vector<MenuSelectOptionElement *>::iterator it;
  for (it = options.begin(); it != options.end(); it++) {
    delete *it;
  }
  options.clear();
}

void MenuSelectOption::enterFocusFrom(int dir) {
  focus = true;
  if (dir == 2) { // bottom
    pointer = size() - 1;
  }
  else {
    pointer = 0;
  }
}

unsigned int MenuSelectOption::size() {
  return options.size();
}
