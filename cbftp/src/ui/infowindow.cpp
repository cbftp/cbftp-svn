#include "infowindow.h"

#include "ui.h"
#include "termint.h"

InfoWindow::InfoWindow(Ui* ui, WINDOW * window, int row, int col) : UIWindow(ui, "InfoWindow") {
  this->window = window;
  split = false;
  label = "";
  text = "";
  init(row, col);
}

void InfoWindow::redraw() {
  ui->erase(window);
  ui->printChar(0, 1, BOX_CORNER_TR, false, window);
  ui->printChar(0, 0, BOX_HLINE, false, window);
  ui->printChar(1, 1, BOX_CORNER_BL, false, window);
  ui->printChar(0, col - 1, BOX_HLINE, false, window);
  ui->printChar(1, col - 2, BOX_CORNER_BR, false, window);
  ui->printChar(0, col - 2, BOX_CORNER_TL, false, window);
  for (unsigned int i = 2; i < col - 2; i++) {
    ui->printChar(1, i, BOX_HLINE, false, window);
  }
  if (split) {
    ui->printChar(1, col / 2, BOX_HLINE_BOT, false, window);
  }
  update();
}

void InfoWindow::update() {
  for (unsigned int i = 2; i < col - 2; i++) {
    ui->printChar(0, i, ' ', false, window);
  }
  unsigned int labellen = label.length();
  ui->printStr(0, 4, label, false, labellen, false, window);
  ui->printStr(0, 4 + labellen + 2, text, false, col - 4 - 4 - labellen - 2, true, window);
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
