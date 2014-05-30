#include "infowindow.h"

#include "ui.h"

InfoWindow::InfoWindow(Ui * ui, WINDOW * window, int row, int col) {
  this->ui = ui;
  this->window = window;
  label = "";
  text = "";
  init(row, col);
}

void InfoWindow::redraw() {
  ui->erase(window);
  ui->printChar(window, 0, 1, 4194411);
  ui->printChar(window, 0, 0, 4194417);
  ui->printChar(window, 1, 1, 4194413);
  ui->printChar(window, 0, col - 1, 4194417);
  ui->printChar(window, 1, col - 2, 4194410);
  ui->printChar(window, 0, col - 2, 4194412);
  for (unsigned int i = 2; i < col - 2; i++) {
    ui->printChar(window, 1, i, 4194417);
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
