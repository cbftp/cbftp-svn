#include "legendwindow.h"
#include <iostream>

LegendWindow::LegendWindow(WINDOW * window, int row, int col) {
  text = "";
  init(window, row, col);
}

void LegendWindow::redraw() {
  werase(window);
  currentpos = 0;
  TermInt::printChar(window, 0, 1, 4194412);
  TermInt::printChar(window, 1, 0, 4194417);
  TermInt::printChar(window, 1, 1, 4194410);
  TermInt::printChar(window, 1, col - 1, 4194417);
  TermInt::printChar(window, 1, col - 2, 4194413);
  TermInt::printChar(window, 0, col - 2, 4194411);
  for (int i = 2; i < col - 2; i++) {
    TermInt::printChar(window, 0, i, 4194417);
  }
  update();
}

void LegendWindow::update() {
  int printpos = 4;
  int textlen = text.length();
  if (textlen > 0) {
    int internalpos = printpos - currentpos++;
    if (currentpos >= textlen) currentpos = 0;
    while (printpos < col - 4) {
      while (printpos - internalpos < textlen && printpos < col - 4) {
        TermInt::printChar(window, 1, printpos, text[printpos - internalpos]);
        ++printpos;
      }
      internalpos = printpos;
    }
  }
}

void LegendWindow::setText(std::string text) {
  this->text = text + "  ::  ";
  currentpos = 0;
  update();

}
