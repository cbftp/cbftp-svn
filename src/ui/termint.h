#pragma once

#include <ncurses.h>
#include <string>

class TermInt {
private:
  static unsigned int cursorrow;
  static unsigned int cursorcol;
  static WINDOW * cursorwindow;
public:
  static void printChar(WINDOW *, unsigned int, unsigned int, unsigned int);
  static void printStr(WINDOW *, unsigned int, unsigned int, std::string);
  static void printStr(WINDOW *, unsigned int, unsigned int, std::string, unsigned int);
  static void printStr(WINDOW *, unsigned int, unsigned int, std::string, unsigned int, bool);
  static void moveCursor(WINDOW *, unsigned int, unsigned int);
};

