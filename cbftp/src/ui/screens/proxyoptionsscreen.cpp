#include "proxyoptionsscreen.h"

#include "../ui.h"
#include "../menuselectoptiontextarrow.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextbutton.h"

#include "../../globalcontext.h"
#include "../../proxy.h"
#include "../../proxymanager.h"
#include "../../sitemanager.h"
#include "../../eventlog.h"
#include "../../util.h"

ProxyOptionsScreen::ProxyOptionsScreen(Ui * ui) {
  this->ui = ui;
}

ProxyOptionsScreen::~ProxyOptionsScreen() {

}

void ProxyOptionsScreen::initialize(unsigned int row, unsigned int col) {
  active = false;
  defaultset = false;
  deleteproxy = "";
  editproxy = "";
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  pm = global->getProxyManager();
  unsigned int y = 1;
  unsigned int x = 1;
  mso.clear();
  useproxy = mso.addTextArrow(y++, x, "useproxy", "Default proxy:");
  focusedarea = &mso;
  mso.makeLeavableDown();
  msop.makeLeavableUp();
  mso.enterFocusFrom(0);
  init(row, col);
}

void ProxyOptionsScreen::redraw() {
  ui->erase();
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
  ui->printStr(4, x, "Name");
  ui->printStr(4, x + 10, "Address");
  ui->printStr(4, x + 30, "Port");
  ui->printStr(4, x + 37, "Auth");
  for(std::vector<Proxy *>::const_iterator it = pm->begin(); it != pm->end(); it++) {
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
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  for (unsigned int i = 0; i < msop.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = msop.getElement(i);
    highlight = false;
    if (msop.isFocused() && msop.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), 9, highlight);
    if (msoe->getLabelText() == msoe->getIdentifier()) {
      Proxy * proxy = pm->getProxy(msoe->getLabelText());
      if (proxy != NULL) {
        ui->printStr(msoe->getRow(), msoe->getCol() + 10, proxy->getAddr(), (unsigned int) 19);
        ui->printStr(msoe->getRow(), msoe->getCol() + 30, proxy->getPort(), (unsigned int) 5);
        ui->printStr(msoe->getRow(), msoe->getCol() + 37, proxy->getAuthMethodText());
      }
    }
  }
}

void ProxyOptionsScreen::update() {
  if (defocusedarea != NULL) {
    if (defocusedarea == &mso) {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    }
    else if (defocusedarea == &msop) {
      std::shared_ptr<MenuSelectOptionElement> msoe = msop.getElement(mso.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    }
  }
  if (focusedarea == &mso) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
    msoe = mso.getElement(mso.getSelectionPointer());
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  else if (focusedarea == &msop) {
    std::shared_ptr<MenuSelectOptionElement> msoe = msop.getElement(msop.getLastSelectionPointer());
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), (unsigned int) 9);
    msoe = msop.getElement(msop.getSelectionPointer());
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), 9, true);
  }
}

void ProxyOptionsScreen::command(const std::string & command) {
  if (command == "yes") {
    if (deleteproxy != "") {
      pm->removeProxy(deleteproxy);
      deleteproxy = "";
      redraw();
      return;
    }
  }
}

bool ProxyOptionsScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  std::shared_ptr<MenuSelectOptionElement> selected;
  switch(ch) {
    case KEY_UP:
      if (focusedarea->goUp()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mso;
          focusedarea->enterFocusFrom(2);
        }
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (focusedarea->goDown()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &msop;
          focusedarea->enterFocusFrom(0);
        }
        ui->update();
      }
      return true;
    case 10:
      selected = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (selected->getIdentifier() == "add") {
        ui->goAddProxy();
        return true;
      }
      activation = focusedarea->activateSelected();
      if (!activation) {
        if (focusedarea == &msop) {
          ui->goEditProxy(selected->getLabelText());
          return true;
        }
        ui->update();
        return true;
      }
      active = true;
      activeelement = selected;
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      return true;
    case 'E':
      selected = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (focusedarea == &msop) {
        if (selected->getIdentifier() == selected->getLabelText()) {
          editproxy = selected->getIdentifier();
          ui->goEditProxy(selected->getLabelText());
          return true;
        }
      }
      return true;
    case 'd':
      for(unsigned int i = 0; i < mso.size(); i++) {
        std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "useproxy") {
          pm->setDefaultProxy(std::static_pointer_cast<MenuSelectOptionTextArrow>(msoe)->getDataText());
        }
      }
      ui->returnToLast();
      return true;
    case KEY_DC:
    case 'D':
      selected = focusedarea->getElement(focusedarea->getSelectionPointer());
      if (focusedarea == &msop && selected->getLabelText() == selected->getIdentifier()) {
        editproxy = selected->getIdentifier();
        deleteproxy = focusedarea->getElement(focusedarea->getSelectionPointer())->getLabelText();
        ui->goConfirmation("Do you really want to delete " + editproxy);
      }
      return true;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string ProxyOptionsScreen::getLegendText() const {
  return currentlegendtext;
}

std::string ProxyOptionsScreen::getInfoLabel() const {
  return "PROXY OPTIONS";
}

std::string ProxyOptionsScreen::getInfoText() const {
  return "Proxies added: " + util::int2Str(pm->size());
}
