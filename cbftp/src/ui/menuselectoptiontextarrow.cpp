#include "menuselectoptiontextarrow.h"

#include <ncursesw/curses.h>

MenuSelectOptionTextArrow::MenuSelectOptionTextArrow(std::string identifier, int row, int col, std::string label) {
  arrow = TextArrow();
  init(identifier, row, col, label);
}

std::string MenuSelectOptionTextArrow::getContentText() const {
  return arrow.getVisual();
}

void MenuSelectOptionTextArrow::inputChar(int ch) {
  switch(ch) {
    case KEY_DOWN:
    case KEY_LEFT:
      arrow.previous();
      break;
    case KEY_UP:
    case KEY_RIGHT:
      arrow.next();
      break;
  }
}

bool MenuSelectOptionTextArrow::activate() {
  arrow.activate();
  return true;
}

void MenuSelectOptionTextArrow::deactivate() {
  arrow.deactivate();
}

bool MenuSelectOptionTextArrow::isActive() const {
  return arrow.isActive();
}

int MenuSelectOptionTextArrow::getData() const {
  return arrow.getOption();
}

std::string MenuSelectOptionTextArrow::getDataText() const {
  return arrow.getOptionText();
}

std::string MenuSelectOptionTextArrow::getLegendText() const {
  return "[Enter] Finish editing - [Left] Previous value - [Right] Next value";
}

void MenuSelectOptionTextArrow::addOption(std::string text, int id) {
  arrow.addOption(text, id);
}

bool MenuSelectOptionTextArrow::setOption(int id) {
  return arrow.setOption(id);
}

bool MenuSelectOptionTextArrow::setOptionText(std::string text) {
  return arrow.setOptionText(text);
}

void MenuSelectOptionTextArrow::clear() {
  arrow.clear();
}

unsigned int MenuSelectOptionTextArrow::wantedWidth() const {
  return arrow.getVisual().length();
}
