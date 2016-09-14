#include "uiwindow.h"

#include "../util.h"

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

void UIWindow::command(const std::string & command) {
  this->command(command, "");
}

void UIWindow::command(const std::string & command, const std::string & arg) {

}

bool UIWindow::keyPressed(unsigned int key) {
  return false;
}

std::string UIWindow::getInfoLabel() const {
  return "";
}

std::string UIWindow::getInfoText() const {
  return "";
}

std::string UIWindow::getLegendText() const {
  return "";
}

bool UIWindow::autoUpdate() const {
  return autoupdate;
}

bool UIWindow::expectBackendPush() const {
  return expectbackendpush;
}
