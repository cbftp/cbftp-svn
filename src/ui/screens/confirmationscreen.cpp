#include "confirmationscreen.h"

#include "../ui.h"
#include "../textwrap.h"

ConfirmationScreen::ConfirmationScreen(Ui * ui) : mode(ConfirmationMode::NORMAL), strongconfirmstep(0) {
  this->ui = ui;
}

void ConfirmationScreen::initialize(unsigned int row, unsigned int col, const std::string & message, ConfirmationMode mode) {
  this->message = message;
  this->mode = mode;
  strongconfirmstep = 0;
  init(row, col);
}

void ConfirmationScreen::redraw() {
  ui->erase();
  std::list<std::string> text = TextWrap::wrap(mode == ConfirmationMode::NORMAL ? (message + " (y/N)?") : message, col - 2);
  unsigned int y = 1;
  for (const std::string & line : text) {
    ui->printStr(y++, 1, line);
  }
  if (mode == ConfirmationMode::STRONG) {
    ui->printStr(y++, 1, "WARNING! Strong confirmation required. Please type \"yes\" to confirm.");
  }
}

bool ConfirmationScreen::keyPressed(unsigned int ch) {
  switch (mode) {
    case ConfirmationMode::INFO:
      ui->returnToLast();
      break;
    case ConfirmationMode::NORMAL:
      if (ch == 'y' || ch == 'Y') {
        ui->confirmYes();
      }
      else {
        ui->confirmNo();
      }
      break;
    case ConfirmationMode::STRONG:
      if (strongconfirmstep == 0 && (ch == 'y' || ch == 'Y')) {
        ++strongconfirmstep;
      }
      else if (strongconfirmstep == 1 && (ch == 'e' || ch == 'E')) {
        ++strongconfirmstep;
      }
      else if (strongconfirmstep == 2 && (ch == 's' || ch == 'S')) {
        ui->confirmYes();
      }
      else {
        ui->confirmNo();
      }
      break;
  }
  return true;
}

std::string ConfirmationScreen::getLegendText() const {
  switch (mode) {
    case ConfirmationMode::INFO:
      return "[Any] return";
    case ConfirmationMode::NORMAL:
      return "[y]es - [Any] No";
    case ConfirmationMode::STRONG:
      return "[yes] - [Any] No";
  }
  return "";
}

std::string ConfirmationScreen::getInfoLabel() const {
  switch (mode) {
    case ConfirmationMode::INFO:
      return "INFO";
    case ConfirmationMode::NORMAL:
      return "CONFIRM YOUR CHOICE";
    case ConfirmationMode::STRONG:
      return "WARNING - STRONG CONFIRMATION REQUIRED";
  }
  return "";
}
