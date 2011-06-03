#include "ui.h"

UserInterface::UserInterface() {
}

bool UserInterface::init() {
  initscr();
  getmaxyx(stdscr, row, col);
  if (row < 24 || col < 80) {
    kill();
    printf("Error: terminal too small. 80x24 required. (Current %dx%d)\n", col, row);
    return false;
  }
  loginscreen = newwin(row, col, 0, 0);
  wrefresh(loginscreen);
  return true;
}

void UserInterface::kill() {
  endwin();
}
