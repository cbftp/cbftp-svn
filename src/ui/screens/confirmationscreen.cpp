#include "confirmationscreen.h"

#include "../uicommunicator.h"
#include "../termint.h"

ConfirmationScreen::ConfirmationScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  init(window, row, col);
}

void ConfirmationScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "Are you sure (y/N)? ");
}

void ConfirmationScreen::keyPressed(unsigned int ch) {
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

std::string ConfirmationScreen::getInfoLabel() {
  return "CONFIRM YOUR CHOICE";
}
