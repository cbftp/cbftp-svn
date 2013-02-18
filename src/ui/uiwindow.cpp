#include "uiwindow.h"

void UIWindow::init(WINDOW * window, unsigned int row, unsigned int col) {
  this->window = window;
  resize(row, col);
}

UIWindow::UIWindow() {
  autoupdate = false;
}

UIWindow::~UIWindow() {

}

void UIWindow::resize(unsigned int row, unsigned int col) {
  this->row = row;
  this->col = col;
  redraw();
}

void UIWindow::update() {

}

void UIWindow::keyPressed(unsigned int key) {

}

std::string UIWindow::getLegendText() {
  return "";
}

bool UIWindow::autoUpdate() {
  return autoupdate;
}
