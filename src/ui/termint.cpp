#include "termint.h"

#include "misc.h"
#include "ncurseswrap.h"

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

void TermInt::printStr(WINDOW * window, unsigned int row, unsigned int col, const FmtString& str) {
  printStr(window, row, col, str, str.length());
}

void TermInt::printStr(WINDOW * window, unsigned int row, unsigned int col, const FmtString& str, unsigned int maxlen) {
  printStr(window, row, col, str, maxlen, false);
}

void TermInt::printStr(WINDOW * window, unsigned int row, unsigned int col, const FmtString& str, unsigned int maxlen, bool rightalign) {
  unsigned int len = str.length();
  unsigned int rawlen = str.rawLength();
  if (len > maxlen) {
    len = maxlen;
  }
  int rightadjust = 0;
  if (rightalign) {
    rightadjust = maxlen - len;
  }
  bool bold = false;
  unsigned int writepos = 0;
  for (unsigned int i = 0; i < rawlen; i++) {
    if (rawlen - i > 3 && str[i] == '%') {
      if (str[i+1] == 'C' && str[i+2] == '(') {
        if (str[i+3] == ')') {
          wattroff(window, COLOR_PAIR(encodeColorRepresentation()));
          i += 3;
          continue;
        }
        else if (rawlen - i > 4 && str[i+4] == ')') {
          int arg = atoi(reinterpret_cast<const char*>(str.data() + i + 3));
          wattron(window, COLOR_PAIR(encodeColorRepresentation(arg)));
          i += 4;
          continue;
        }
        else if (rawlen - i > 6 && str[i+4] == ',' && str[i+6] == ')') {
          int arg1 = atoi(reinterpret_cast<const char*>(str.data() + i + 3));
          int arg2 = atoi(reinterpret_cast<const char*>(str.data() + i + 5));
          wattron(window, COLOR_PAIR(encodeColorRepresentation(arg1, arg2)));
          i += 6;
          continue;
        }
      }
      else if (str[i+1] == 'B' && str[i+2] == '(' && str[i+3] == ')') {
        bold ^= 1;
        if (bold) {
          wattron(window, A_BOLD);
        }
        else {
          wattroff(window, A_BOLD);
        }
        i += 3;
        continue;
      }
    }
    if (writepos >= len) {
      break;
    }
    printChar(window, row, col + writepos++ + rightadjust, str[i]);
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

