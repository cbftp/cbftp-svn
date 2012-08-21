#include "termint.h"
#include <iostream>
void TermInt::printChar(WINDOW * window, unsigned int row, unsigned int col, unsigned int c) {
  mvwaddch(window, row, col, c);
  wmove(cursorwindow, cursorrow, cursorcol);
}

void TermInt::printStr(WINDOW * window, unsigned int row, unsigned int col, std::string str) {
  mvwprintw(window, row, col, str.c_str());
  wmove(cursorwindow, cursorrow, cursorcol);
}

void TermInt::printStr(WINDOW * window, unsigned int row, unsigned int col, std::string str, unsigned int maxlen) {
  unsigned int len = str.length();
  if (len > maxlen) {
    len = maxlen;
  }
  for (unsigned int i = 0; i < len; i++) {
    mvwaddch(window, row, col + i, (unsigned int)str[i]);
  }
  wmove(cursorwindow, cursorrow, cursorcol);
}

void TermInt::moveCursor(WINDOW * window, unsigned int row, unsigned int col) {
  cursorrow = row;
  cursorcol = col;
  cursorwindow = window;
  wmove(window, row, col);
}

unsigned int TermInt::cursorrow = 0;
unsigned int TermInt::cursorcol = 0;
WINDOW * TermInt::cursorwindow = stdscr;
