#include "uiwindow.h"

#include "ui.h"

void UIWindow::init(unsigned int row, unsigned int col) {
  resize(row, col);
  redraw();
}

UIWindow::UIWindow(Ui* ui, const std::string& name) : name(name), row(0), col(0),
    autoupdate(false), expectbackendpush(false), ui(ui), keybinds(name), allowimplicitgokeybinds(true)
{
  ui->addKeyBinds(&keybinds);
}

UIWindow::~UIWindow() {
  ui->removeKeyBinds(&keybinds);
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

bool UIWindow::keyPressedBase(unsigned int key) {
  int action = keybinds.getKeyAction(key);
  if (action == KEYACTION_KEYBINDS && allowimplicitgokeybinds) {
    ui->goKeyBinds(&keybinds);
    return true;
  }
  return keyPressed(key);
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
  return keybinds.getLegendSummary();
}

bool UIWindow::autoUpdate() const {
  return autoupdate;
}

bool UIWindow::expectBackendPush() const {
  return expectbackendpush;
}
