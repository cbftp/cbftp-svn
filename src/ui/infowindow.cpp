#include "infowindow.h"

#include "ui.h"
#include "termint.h"

InfoWindow::InfoWindow(Ui * ui, WINDOW * window, int row, int col) {
  this->ui = ui;
  this->window = window;
  split = false;
  label = "";
  text = "";
  init(row, col);
}

void InfoWindow::redraw() {
  ui->erase(window);
  ui->printChar(window, 0, 1, BOX_CORNER_TR);
  ui->printChar(window, 0, 0, BOX_HLINE);
  ui->printChar(window, 1, 1, BOX_CORNER_BL);
  ui->printChar(window, 0, col - 1, BOX_HLINE);
  ui->printChar(window, 1, col - 2, BOX_CORNER_BR);
  ui->printChar(window, 0, col - 2, BOX_CORNER_TL);
  for (unsigned int i = 2; i < col - 2; i++) {
    ui->printChar(window, 1, i, BOX_HLINE);
  }
  if (split) {
    ui->printChar(window, 1, col / 2, BOX_HLINE_BOT);
  }
  update();
}

void InfoWindow::update() {
  for (unsigned int i = 2; i < col - 2; i++) {
    ui->printChar(window, 0, i, ' ');
  }
  unsigned int labellen = label.length();
  ui->printStr(window, 0, 4, label);
  ui->printStr(window, 0, 4 + labellen + 2, text, col - 4 - 4 - labellen - 2, false, true);
}

void InfoWindow::setLabel(std::string label) {
  this->label = label;
  update();
}
void InfoWindow::setText(std::string text) {
  this->text = text;
  update();

}

void InfoWindow::setSplit(bool split) {
  this->split = split;
}
