#include "confirmationscreen.h"

ConfirmationScreen::ConfirmationScreen(WINDOW * window, UICommunicator * uicommunicator, int row, int col) {
  this->uicommunicator = uicommunicator;
  init(window, row, col);
}

void ConfirmationScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "CONFIRM YOUR CHOICE");
  TermInt::printStr(window, 3, 1, "Are you sure (y/N)? ");
}

void ConfirmationScreen::keyPressed(int ch) {
  if (ch == 'y') {
    uicommunicator->newCommand("yes");
  }
  else {
    uicommunicator->newCommand("no");
  }
}

std::string ConfirmationScreen::getLegendText() {
  return "[y]es - [Any] No";
}
