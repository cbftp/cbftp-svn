#include "termint.h"
#include <iostream>
void TermInt::printChar(WINDOW * window, int row, int col, int c) {
  mvwaddch(window, row, col, c);
  wmove(cursorwindow, cursorrow, cursorcol);
}

void TermInt::printStr(WINDOW * window, int row, int col, std::string str) {
  mvwprintw(window, row, col, str.c_str());
  wmove(cursorwindow, cursorrow, cursorcol);
}

void TermInt::moveCursor(WINDOW * window, int row, int col) {
  cursorrow = row;
  cursorcol = col;
  cursorwindow = window;
  wmove(window, row, col);
}

int TermInt::cursorrow = 0;
int TermInt::cursorcol = 0;
WINDOW * TermInt::cursorwindow = stdscr;
