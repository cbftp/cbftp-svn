#include "uiwindow.h"

void UIWindow::init(WINDOW * window, int row, int col) {
  this->window = window;
  resize(row, col);
}

UIWindow::~UIWindow() {

}

void UIWindow::resize(int row, int col) {
  this->row = row;
  this->col = col;
  redraw();
}

void UIWindow::update() {

}

void UIWindow::keyPressed(int key) {

}

std::string UIWindow::getLegendText() {
  return "";
}
