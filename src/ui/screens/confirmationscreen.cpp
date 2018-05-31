#include "confirmationscreen.h"

#include "../ui.h"

ConfirmationScreen::ConfirmationScreen(Ui * ui) : strong(false), strongconfirmstep(0) {
  this->ui = ui;
}

void ConfirmationScreen::initialize(unsigned int row, unsigned int col, const std::string & message, bool strong) {
  this->message = message;
  this->strong = strong;
  strongconfirmstep = 0;
  init(row, col);
}

void ConfirmationScreen::redraw() {
  ui->erase();
  if (!strong) {
    ui->printStr(1, 1, message + " (y/N)? ");
  }
  else {
    ui->printStr(1, 1, message);
    ui->printStr(2, 1, "WARNING! Strong confirmation required. Please type \"yes\" to confirm.");
  }
}

bool ConfirmationScreen::keyPressed(unsigned int ch) {
  if (strong && strongconfirmstep == 0 && (ch == 'y' || ch == 'Y')) {
    ++strongconfirmstep;
  }
  else if (strong && strongconfirmstep == 1 && (ch == 'e' || ch == 'E')) {
    ++strongconfirmstep;
  }
  else if (strong && strongconfirmstep == 2 && (ch == 's' || ch == 'S')) {
    ui->confirmYes();
  }
  else if (!strong && (ch == 'y' || ch == 'Y')) {
    ui->confirmYes();
  }
  else {
    ui->confirmNo();
  }
  return true;
}

std::string ConfirmationScreen::getLegendText() const {
  if (!strong) {
    return "[y]es - [Any] No";
  }
  return "[yes] - [Any] No";
}

std::string ConfirmationScreen::getInfoLabel() const {
  if (!strong) {
    return "CONFIRM YOUR CHOICE";
  }
  return "WARNING - STRONG CONFIRMATION REQUIRED";
}
