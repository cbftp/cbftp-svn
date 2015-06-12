#include "termint.h"

#ifndef _XOPEN_SOURCE_EXTENDED
#define _XOPEN_SOURCE_EXTENDED
#endif
#include <ncursesw/curses.h>

void TermInt::printChar(WINDOW * window, unsigned int row, unsigned int col, unsigned int c) {
  cchar_t ch = {0, {static_cast<wchar_t>(c)}};

  if (c >= BOX_MIN && c <= BOX_MAX) {
    mvwaddch(window, row, col, c);
  }
  else if (c != '%') {
    mvwadd_wch(window, row, col, &ch);
  }
  else {
    mvwprintw(window, row, col, "%%");
  }
  wmove(cursorwindow, cursorrow, cursorcol);
}

void TermInt::printStr(WINDOW * window, unsigned int row, unsigned int col, std::string str) {
  printStr(window, row, col, str, str.length());
}

void TermInt::printStr(WINDOW * window, unsigned int row, unsigned int col, std::string str, unsigned int maxlen) {
  printStr(window, row, col, str, maxlen, false);
}

void TermInt::printStr(WINDOW * window, unsigned int row, unsigned int col, std::string str, unsigned int maxlen, bool rightalign) {
  unsigned int len = str.length();
  if (len > maxlen) {
    len = maxlen;
  }
  int rightadjust = 0;
  if (rightalign) {
    rightadjust = maxlen - len;
  }
  for (unsigned int i = 0; i < len; i++) {
    printChar(window, row, col + i + rightadjust, (unsigned char)str[i]);
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
