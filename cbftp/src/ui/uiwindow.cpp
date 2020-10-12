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

void UIWindow::command(const std::string& command) {
  this->command(command, "");
}

void UIWindow::command(const std::string& command, const std::string& arg) {

}

bool UIWindow::keyPressedBase(unsigned int key) {
  bool caught = keyPressed(key);
  if (caught) {
    return true;
  }
  int action = keybinds.getKeyAction(key);
  if (action == KEYACTION_KEYBINDS && allowimplicitgokeybinds) {
    ui->goKeyBinds(&keybinds);
    return true;
  }
  return false;
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

std::basic_string<unsigned int> UIWindow::getWideInfoLabel() const {
  std::string label = getInfoLabel();
  return std::basic_string<unsigned int>(label.begin(), label.end());
}

std::basic_string<unsigned int> UIWindow::getWideInfoText() const {
  std::string text = getInfoText();
  return std::basic_string<unsigned int>(text.begin(), text.end());
}

std::basic_string<unsigned int> UIWindow::getWideLegendText() const {
  std::string text = getLegendText();
  return std::basic_string<unsigned int>(text.begin(), text.end());
}

bool UIWindow::autoUpdate() const {
  return autoupdate;
}

bool UIWindow::expectBackendPush() const {
  return expectbackendpush;
}

bool UIWindow::isTop() const {
  return ui->isTop(this);
}
