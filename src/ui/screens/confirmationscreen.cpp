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

void ConfirmationScreen::keyPressed(unsigned int ch) {
  if (ch == 'y') {
    ui->confirmYes();
  }
  else {
    ui->confirmNo();
  }
}

std::string ConfirmationScreen::getLegendText() {
  return "[y]es - [Any] No";
}

std::string ConfirmationScreen::getInfoLabel() {
  return "CONFIRM YOUR CHOICE";
}
