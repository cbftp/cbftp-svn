#include "menuselectoptiontextbutton.h"

MenuSelectOptionTextButton::MenuSelectOptionTextButton(std::string identifier, int row, int col, std::string text) {
  this->text = text;
  init(identifier, row, col, text);
}
MenuSelectOptionTextButton::MenuSelectOptionTextButton(std::string identifier, int row, int col, std::string text, bool content) {
  if (content) {
    this->text = text;
  }
  else {
    this->text = "";
  }
  init(identifier, row, col, text);
}

std::string MenuSelectOptionTextButton::getContentText() {
  return text;
}

bool MenuSelectOptionTextButton::isActivated() {
  return active;
}

bool MenuSelectOptionTextButton::activate() {
  active = true;
  return false;
}
