#include "uiwindow.h"

void UIWindow::init(unsigned int row, unsigned int col) {
  resize(row, col);
  redraw();
}

UIWindow::UIWindow() {
  autoupdate = false;
  expectbackendpush = false;
}

UIWindow::~UIWindow() {

}

void UIWindow::resize(unsigned int row, unsigned int col) {
  this->row = row;
  this->col = col;
}

void UIWindow::update() {

}

void UIWindow::command(std::string command) {
  this->command(command, "");
}

void UIWindow::command(std::string command, std::string arg) {

}

void UIWindow::keyPressed(unsigned int key) {

}

std::string UIWindow::getInfoLabel() {
  return "";
}

std::string UIWindow::getInfoText() {
  return "";
}

std::string UIWindow::getLegendText() {
  return "";
}

bool UIWindow::autoUpdate() {
  return autoupdate;
}

bool UIWindow::expectBackendPush() {
  return expectbackendpush;
}
