#include "menuselectoptionelement.h"

void MenuSelectOptionElement::init(std::string identifier, int row, int col, std::string label) {
  this->identifier = identifier;
  this->row = row;
  this->col = col;
  this->label = label;
  active = false;
  shown = true;
  selectable = true;
}

MenuSelectOptionElement::~MenuSelectOptionElement() {

}

void MenuSelectOptionElement::setPosition(int row, int col) {
  this->row = row;
  this->col = col;
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

bool MenuSelectOptionElement::isActive() {
  return active;
}

std::string MenuSelectOptionElement::getLegendText() {
  return "";
}

unsigned int MenuSelectOptionElement::getCol() {
  return col;
}

unsigned int MenuSelectOptionElement::getRow() {
  return row;
}

int MenuSelectOptionElement::cursorPosition() {
  return -1;
}

void MenuSelectOptionElement::inputChar(int ch) {

}

void MenuSelectOptionElement::hide() {
  shown = false;
}

void MenuSelectOptionElement::show() {
  shown = true;
}

bool MenuSelectOptionElement::visible() {
  return shown;
}

bool MenuSelectOptionElement::isSelectable() {
  return selectable;
}
