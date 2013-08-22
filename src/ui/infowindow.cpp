#include "infowindow.h"
#include <iostream>

InfoWindow::InfoWindow(WINDOW * window, int row, int col) {
  label = "";
  text = "";
  init(window, row, col);
}

void InfoWindow::redraw() {
  werase(window);
  TermInt::printChar(window, 0, 1, 4194411);
  TermInt::printChar(window, 0, 0, 4194417);
  TermInt::printChar(window, 1, 1, 4194413);
  TermInt::printChar(window, 0, col - 1, 4194417);
  TermInt::printChar(window, 1, col - 2, 4194410);
  TermInt::printChar(window, 0, col - 2, 4194412);
  for (unsigned int i = 2; i < col - 2; i++) {
    TermInt::printChar(window, 1, i, 4194417);
  }
  update();
}

void InfoWindow::update() {
  for (unsigned int i = 2; i < col - 2; i++) {
    TermInt::printChar(window, 0, i, ' ');
  }
  unsigned int labellen = label.length();
  TermInt::printStr(window, 0, 4, label);
  TermInt::printStr(window, 0, 4 + labellen + 2, text, col - 4 - 4 - labellen - 2, true);
}

void InfoWindow::setLabel(std::string label) {
  this->label = label;
  update();
}
void InfoWindow::setText(std::string text) {
  this->text = text;
  update();

}
