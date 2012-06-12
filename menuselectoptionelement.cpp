#include "menuselectoptionelement.h"

void MenuSelectOptionElement::init(std::string identifier, int row, int col, std::string label) {
  this->identifier = identifier;
  this->row = row;
  this->col = col;
  this->label = label;
  active = false;
}

MenuSelectOptionElement::~MenuSelectOptionElement() {

}

std::string MenuSelectOptionElement::getLabelText() {
  return label;
}

std::string MenuSelectOptionElement::getIdentifier() {
  return identifier;
}

bool MenuSelectOptionElement::activate() {
  active = true;
  return true;
}

void MenuSelectOptionElement::deactivate() {
  active = false;
}

int MenuSelectOptionElement::getCol() {
  return col;
}

int MenuSelectOptionElement::getRow() {
  return row;
}

int MenuSelectOptionElement::cursorPosition() {
  return -1;
}

void MenuSelectOptionElement::inputChar(int ch) {

}
