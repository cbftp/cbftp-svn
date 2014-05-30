#include "transfersscreen.h"

#include "../ui.h"

TransfersScreen::TransfersScreen(Ui * ui) {
  this->ui = ui;
}

void TransfersScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  init(row, col);
}

void TransfersScreen::redraw() {
  ui->erase();
  ui->printStr(1, 1, "placeholder");
}

void TransfersScreen::update() {
  redraw();
}

void TransfersScreen::keyPressed(unsigned int ch) {
  switch (ch) {

  }
}

std::string TransfersScreen::getLegendText() {
  return "";
}

std::string TransfersScreen::getInfoLabel() {
  return "TRANSFERS";
}
