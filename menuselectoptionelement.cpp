#include "menuselectoptionelement.h"

MenuSelectOptionElement::MenuSelectOptionElement(int row, int col, std::string identifier, std::string label, std::string starttext) {
  this->row = row;
  this->col = col;
  this->identifier = identifier;
  this->label = label;
  this->strcontent = starttext;
  this->str = true;
}

MenuSelectOptionElement::MenuSelectOptionElement(int row, int col, std::string identifier, std::string label, int startval, bool checkbox) {
  this->row = row;
  this->col = col;
  this->identifier = identifier;
  this->label = label;
  this->intcontent = startval;
  this->str = false;
  this->checkbox = checkbox;
}

std::string MenuSelectOptionElement::getIdentifier() {
  return identifier;
}

std::string MenuSelectOptionElement::getLabel() {
  return label;
}

std::string MenuSelectOptionElement::getContent() {
  return strcontent;
}

int MenuSelectOptionElement::getIntContent() {
  return intcontent;
}

void MenuSelectOptionElement::setIntContent(int inval) {
  intcontent = inval;
}

void MenuSelectOptionElement::setContent(std::string str) {
  strcontent = str;
}

bool MenuSelectOptionElement::hasStrValue() {
  return str;
}

bool MenuSelectOptionElement::isCheckBox() {
  return checkbox;
}

int MenuSelectOptionElement::getCol() {
  return col;
}

int MenuSelectOptionElement::getRow() {
  return row;
}
