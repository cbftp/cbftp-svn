#include "editproxyscreen.h"

#include "../uicommunicator.h"
#include "../termint.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptiontextarrow.h"

#include "../../globalcontext.h"
#include "../../proxymanager.h"

extern GlobalContext * global;

EditProxyScreen::EditProxyScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  active = false;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one, save changes - [c]ancel, undo changes";
  currentlegendtext = defaultlegendtext;
  operation = uicommunicator->getArg1();
  std::string arg2 = uicommunicator->getArg2();
  uicommunicator->checkoutCommand();
  if (operation == "add") {
    modproxy = Proxy("proxy1");
  }
  else if (operation == "edit") {
    proxy = global->getProxyManager()->getProxy(arg2);
    modproxy = Proxy(*proxy);
  }
  unsigned int y = 2;
  unsigned int x = 1;
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
  init(window, row, col);
}

void EditProxyScreen::redraw() {
  werase(window);
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
  TermInt::printStr(window, 1, 1, "Type: SOCKS5");
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    if (!msoe->visible()) {
      continue;
    }
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    if (highlight) wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void EditProxyScreen::update() {
  if (latestauthmethod != authmethod->getData() && !authmethod->isActive()) {
    redraw();
    return;
  }
  MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
  if (msoe->visible()) {
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  msoe = mso.getElement(mso.getSelectionPointer());
  wattron(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  wattroff(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  if (active && msoe->cursorPosition() >= 0) {
    curs_set(1);
    TermInt::moveCursor(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void EditProxyScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      uicommunicator->newCommand("updatesetlegend");
      return;
    }
    activeelement->inputChar(ch);
    uicommunicator->newCommand("update");
    return;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      if (mso.goUp()) {
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_DOWN:
      if (mso.goDown()) {
        uicommunicator->newCommand("update");
      }
      break;
    case 10:
      activation = mso.activateSelected();
      if (!activation) {
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 'd':
      if (operation == "add") {
        proxy = new Proxy();
      }
      for(unsigned int i = 0; i < mso.size(); i++) {
        MenuSelectOptionElement * msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "name") {
          std::string newname = ((MenuSelectOptionTextField *)msoe)->getData();
          proxy->setName(newname);
        }
        else if (identifier == "addr") {
          proxy->setAddr(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "port") {
          proxy->setPort(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "user") {
          proxy->setUser(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "pass") {
          proxy->setPass(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "authmethod") {
          proxy->setAuthMethod(((MenuSelectOptionTextArrow *)msoe)->getData());
        }
      }
      if (operation == "add") {
        global->getProxyManager()->addProxy(proxy);
      }
      else {
        global->getProxyManager()->sortProxys();
      }
      uicommunicator->newCommand("return");
      return;
    case 27: // esc
    case 'c':
      uicommunicator->newCommand("return");
      break;
  }
}

std::string EditProxyScreen::getLegendText() {
  return currentlegendtext;
}

std::string EditProxyScreen::getInfoLabel() {
  return "EDIT PROXY";
}
