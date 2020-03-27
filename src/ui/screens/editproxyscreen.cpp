#include "editproxyscreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptiontextarrow.h"

#include "../../globalcontext.h"
#include "../../proxymanager.h"

EditProxyScreen::EditProxyScreen(Ui* ui) : UIWindow(ui, "EditProxyScreen") {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

EditProxyScreen::~EditProxyScreen() {

}

void EditProxyScreen::initialize(unsigned int row, unsigned int col, std::string operation, std::string proxy) {
  active = false;
  this->operation = operation;
  if (operation == "add") {
    modproxy = Proxy("proxy1");
  }
  else if (operation == "edit") {
    this->proxy = global->getProxyManager()->getProxy(proxy);
    modproxy = Proxy(*this->proxy);
  }
  unsigned int y = 2;
  unsigned int x = 1;
  mso.clear();
  mso.addStringField(y++, x, "name", "Name:", modproxy.getName(), false);
  mso.addStringField(y++, x, "addr", "Address:", modproxy.getAddr(), false);
  mso.addStringField(y++, x, "port", "Port:", modproxy.getPort(), false);
  authmethod = mso.addTextArrow(y++, x, "authmethod", "Auth method:");
  authmethod->addOption("None", PROXY_AUTH_NONE);
  authmethod->addOption("User/pass", PROXY_AUTH_USERPASS);
  authmethod->setOption(modproxy.getAuthMethod());
  mso.addStringField(y++, x, "user", "Username:", modproxy.getUser(), false);
  mso.addStringField(y++, x, "pass", "Password:", modproxy.getPass(), true);
  mso.enterFocusFrom(0);
  init(row, col);
}

void EditProxyScreen::redraw() {
  ui->erase();
  bool highlight;
  if (authmethod->getData() == PROXY_AUTH_NONE) {
    mso.getElement("user")->hide();
    mso.getElement("pass")->hide();
  }
  else {
    mso.getElement("user")->show();
    mso.getElement("pass")->show();
  }
  latestauthmethod = authmethod->getData();
  ui->printStr(1, 1, "Type: SOCKS5");
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    if (!msoe->visible()) {
      continue;
    }
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void EditProxyScreen::update() {
  if (latestauthmethod != authmethod->getData() && !authmethod->isActive()) {
    redraw();
    return;
  }
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  if (msoe->visible()) {
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  msoe = mso.getElement(mso.getSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    ui->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool EditProxyScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      ui->setLegend();
      ui->update();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  switch(action) {
    case KEYACTION_UP:
      if (mso.goUp()) {
        ui->update();
      }
      return true;
    case KEYACTION_DOWN:
      if (mso.goDown()) {
        ui->update();
      }
      return true;
    case KEYACTION_ENTER:
      activation = mso.activateSelected();
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      ui->setLegend();
      ui->update();
      return true;
    case KEYACTION_DONE:
      if (operation == "add") {
        proxy = new Proxy();
      }
      for(unsigned int i = 0; i < mso.size(); i++) {
        std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "name") {
          std::string newname = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
          proxy->setName(newname);
        }
        else if (identifier == "addr") {
          proxy->setAddr(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "port") {
          proxy->setPort(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "user") {
          proxy->setUser(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "pass") {
          proxy->setPass(std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData());
        }
        else if (identifier == "authmethod") {
          proxy->setAuthMethod(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getData());
        }
      }
      if (operation == "add") {
        global->getProxyManager()->addProxy(proxy);
      }
      else {
        global->getProxyManager()->sortProxys();
      }
      ui->returnToLast();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string EditProxyScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string EditProxyScreen::getInfoLabel() const {
  return "EDIT PROXY";
}
