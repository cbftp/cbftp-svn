#include "confirmationscreen.h"

#include "../ui.h"

ConfirmationScreen::ConfirmationScreen(Ui * ui) {
  this->ui = ui;
}

void ConfirmationScreen::initialize(unsigned int row, unsigned int col, std::string message) {
  this->message = message;
  init(row, col);
}

void ConfirmationScreen::redraw() {
  ui->erase();
  ui->printStr(1, 1, message + " (y/N)? ");
}

bool ConfirmationScreen::keyPressed(unsigned int ch) {
  if (ch == 'y' || ch == 'Y') {
    ui->confirmYes();
  }
  else {
    ui->confirmNo();
  }
  return true;
}

std::string ConfirmationScreen::getLegendText() const {
  return "[y]es - [Any] No";
}

std::string ConfirmationScreen::getInfoLabel() const {
  return "CONFIRM YOUR CHOICE";
}
