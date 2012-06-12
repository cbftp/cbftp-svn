#include "confirmationscreen.h"

ConfirmationScreen::ConfirmationScreen(WINDOW * window, UIWindowCommand * windowcommand, int row, int col) {
  this->windowcommand = windowcommand;
  init(window, row, col);
}

void ConfirmationScreen::redraw() {
  werase(window);
  mvwprintw(window, 1, 1, "CONFIRM YOUR CHOICE");
  mvwprintw(window, 3, 1, "Are you sure (y/N)? ");
}

void ConfirmationScreen::keyPressed(int ch) {
  if (ch == 'y') {
    windowcommand->newCommand("yes");
    std::cout << "MJEHEZ" << std::endl;
  }
  else {
    windowcommand->newCommand("no");
  }
}




