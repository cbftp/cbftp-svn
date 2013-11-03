#include "proxyoptionsscreen.h"

#include "../uicommunicator.h"
#include "../termint.h"
#include "../menuselectoptiontextarrow.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextbutton.h"

#include "../../globalcontext.h"
#include "../../proxy.h"
#include "../../proxymanager.h"
#include "../../sitemanager.h"
#include "../../eventlog.h"

extern GlobalContext * global;

ProxyOptionsScreen::ProxyOptionsScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  active = false;
  defaultset = false;
  deleteproxy = "";
  editproxy = "";
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  pm = global->getProxyManager();
  unsigned int y = 1;
  unsigned int x = 1;
  useproxy = mso.addTextArrow(y++, x, "useproxy", "Default proxy:");
  focusedarea = &mso;
  mso.makeLeavableDown();
  msop.makeLeavableUp();
  mso.enterFocusFrom(0);
  init(window, row, col);
}

void ProxyOptionsScreen::redraw() {
  werase(window);
  if (editproxy != "" && pm->getProxy(editproxy) == NULL) {
    global->getSiteManager()->proxyRemoved(editproxy);
    editproxy = "";
  }
  std::string prevselect = useproxy->getDataText();
  useproxy->clear();
  useproxy->addOption("None", 0);
  unsigned int y = 6;
  unsigned int x = 1;
  msop.clear();
  msop.addTextButtonNoContent(2, x, "add", "Add proxy");
  TermInt::printStr(window, 4, x, "Name");
  TermInt::printStr(window, 4, x + 10, "Address");
  TermInt::printStr(window, 4, x + 30, "Port");
  TermInt::printStr(window, 4, x + 37, "Auth");
  for(std::vector<Proxy *>::iterator it = pm->begin(); it != pm->end(); it++) {
    std::string name = (*it)->getName();
    useproxy->addOption(name, 1);
    msop.addTextButton(y++, x, name, name);
  }
  msop.checkPointer();
  if (!defaultset) {
    if (pm->getDefaultProxy() != NULL) {
      useproxy->setOptionText(pm->getDefaultProxy()->getName());
    }
    defaultset = true;
  }
  else {
    useproxy->setOptionText(prevselect);
  }
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    if (highlight) wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  for (unsigned int i = 0; i < msop.size(); i++) {
    MenuSelectOptionElement * msoe = msop.getElement(i);
    highlight = false;
    if (msop.isFocused() && msop.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText(), 9);
    if (highlight) wattroff(window, A_REVERSE);
    if (msoe->getLabelText() == msoe->getIdentifier()) {
      Proxy * proxy = pm->getProxy(msoe->getLabelText());
      if (proxy != NULL) {
        TermInt::printStr(window, msoe->getRow(), msoe->getCol() + 10, proxy->getAddr(), 19);
        TermInt::printStr(window, msoe->getRow(), msoe->getCol() + 30, proxy->getPort(), 5);
        TermInt::printStr(window, msoe->getRow(), msoe->getCol() + 37, proxy->getAuthMethodText());
      }
    }
  }
}

void ProxyOptionsScreen::update() {
  if (uicommunicator->hasNewCommand()) {
    if (uicommunicator->getCommand() == "yes") {
      uicommunicator->checkoutCommand();
      if (deleteproxy != "") {
        pm->removeProxy(deleteproxy);
        deleteproxy = "";
        redraw();
        return;
      }
    }
    uicommunicator->checkoutCommand();
  }
  if (defocusedarea != NULL) {
    if (defocusedarea == &mso) {
      MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
      TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    }
    else if (defocusedarea == &msop) {
      MenuSelectOptionElement * msoe = msop.getElement(mso.getLastSelectionPointer());
      TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    }
  }
  if (focusedarea == &mso) {
    MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
    msoe = mso.getElement(mso.getSelectionPointer());
    wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  else if (focusedarea == &msop) {
    MenuSelectOptionElement * msoe = msop.getElement(msop.getLastSelectionPointer());
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText(), 9);
    msoe = msop.getElement(msop.getSelectionPointer());
    wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText(), 9);
    wattroff(window, A_REVERSE);
  }
}

void ProxyOptionsScreen::keyPressed(unsigned int ch) {
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
  MenuSelectOptionElement * selected;
  switch(ch) {
    case KEY_UP:
      if (focusedarea->goUp()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mso;
          focusedarea->enterFocusFrom(2);
        }
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_DOWN:
      if (focusedarea->goDown()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &msop;
          focusedarea->enterFocusFrom(0);
        }
        uicommunicator->newCommand("update");
      }
      break;
    case 10:
      selected = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (selected->getIdentifier() == "add") {
        uicommunicator->newCommand("editproxy", "add");
        break;
      }
      activation = focusedarea->activateSelected();
      if (!activation) {
        if (focusedarea == &msop) {
          uicommunicator->newCommand("editproxy", "edit", selected->getLabelText());
          break;
        }
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = selected;
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 'E':
      selected = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (focusedarea == &msop) {
        if (selected->getIdentifier() == selected->getLabelText()) {
          editproxy = selected->getIdentifier();
          uicommunicator->newCommand("editproxy", "edit", selected->getLabelText());
          break;
        }
      }
      break;
    case 'd':
      for(unsigned int i = 0; i < mso.size(); i++) {
        MenuSelectOptionElement * msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "useproxy") {
          pm->setDefaultProxy(((MenuSelectOptionTextArrow *)msoe)->getDataText());
        }
      }
      uicommunicator->newCommand("return");
      break;
    case KEY_DC:
    case 'D':
      selected = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (focusedarea == &msop && selected->getLabelText() == selected->getIdentifier()) {
        editproxy = selected->getIdentifier();
        deleteproxy = focusedarea->getElement(focusedarea->getSelectionPointer())->getLabelText();
        uicommunicator->newCommand("confirmation");
      }
      break;
    case 27: // esc
    case 'c':
      uicommunicator->newCommand("return");
      break;
  }
}

std::string ProxyOptionsScreen::getLegendText() {
  return currentlegendtext;
}

std::string ProxyOptionsScreen::getInfoLabel() {
  return "PROXY OPTIONS";
}

std::string ProxyOptionsScreen::getInfoText() {
  return "Proxies added: " + global->int2Str(pm->size());
}
