#include "menuselectoptionnumarrow.h"

#include "ncurseswrap.h"

MenuSelectOptionNumArrow::MenuSelectOptionNumArrow(std::string identifier, int row, int col, std::string label, int value, int min, int max) {
  arrow = NumInputArrow(value, min, max);
  init(identifier, row, col, label);
}

std::string MenuSelectOptionNumArrow::getContentText() const {
  return arrow.getVisual();
}

void MenuSelectOptionNumArrow::inputChar(int ch) {
  switch(ch) {
    case KEY_DOWN:
    case KEY_LEFT:
      arrow.decrease();
      break;
    case KEY_UP:
    case KEY_RIGHT:
      arrow.increase();
      break;
  }
}

bool MenuSelectOptionNumArrow::activate() {
  arrow.activate();
  return true;
}

void MenuSelectOptionNumArrow::deactivate() {
  arrow.deactivate();
}

int MenuSelectOptionNumArrow::getData() const {
  return arrow.getValue();
}

std::string MenuSelectOptionNumArrow::getLegendText() const {
  return "[Enter] Finish editing - [Left] Decrease value - [Right] Increase value";
}
