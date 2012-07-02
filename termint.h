#pragma once
#include <ncurses.h>
#include <string>

class TermInt {
private:
  static int cursorrow;
  static int cursorcol;
  static WINDOW * cursorwindow;
public:
  static void printChar(WINDOW *, int, int, int);
  static void printStr(WINDOW *, int, int, std::string);
  static void moveCursor(WINDOW *, int, int);
};

