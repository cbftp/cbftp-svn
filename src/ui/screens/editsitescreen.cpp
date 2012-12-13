#include "editsitescreen.h"

EditSiteScreen::EditSiteScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  active = false;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one, save changes - [c]ancel, undo changes";
  currentlegendtext = defaultlegendtext;
  operation = uicommunicator->getArg1();
  std::string arg2 = uicommunicator->getArg2();
  uicommunicator->checkoutCommand();
  if (operation == "add") {
    modsite = Site("SUNET");
  }
  else if (operation == "edit") {
    site = global->getSiteManager()->getSite(arg2);
    modsite = Site(*site);
  }
  unsigned int y = 3;
  unsigned int x = 1;

  mso.addStringField(y++, x, "name", "Name:", modsite.getName(), false);
  mso.addStringField(y++, x, "addr", "Address:", modsite.getAddress(), false);
  mso.addStringField(y++, x, "port", "Port:", modsite.getPort(), false);
  mso.addStringField(y++, x, "user", "Username:", modsite.getUser(), false);
  mso.addStringField(y++, x, "pass", "Password:", modsite.getPass(), true);
  mso.addIntArrow(y++, x, "logins", "Login slots:", modsite.getInternMaxLogins(), 0, 99);
  mso.addIntArrow(y++, x, "maxup", "Upload slots:", modsite.getInternMaxUp(), 0, 99);
  mso.addIntArrow(y++, x, "maxdn", "Download slots:", modsite.getInternMaxDown(), 0, 99);
  mso.addCheckBox(y++, x, "pret", "Needs PRET:", modsite.needsPRET());
  mso.addCheckBox(y++, x, "brokenpasv", "Broken PASV:", modsite.hasBrokenPASV());
  y++;
  ms.initialize(y++, x, modsite.sectionsBegin(), modsite.sectionsEnd());
  focusedarea = &mso;
  mso.makeLeavableDown();
  ms.makeLeavableUp();
  mso.enterFocusFrom(0);
  init(window, row, col);
}

void EditSiteScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "-== SITE OPTIONS ==-");
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
  int headrow = ms.getHeaderRow();
  int headcol = ms.getHeaderCol();
  TermInt::printStr(window, headrow, headcol, "Sections");
  int selected = ms.getSelectionPointer();
  highlight = false;
  if (ms.isFocused() && selected == 0) {
    highlight = true;
  }
  if (highlight) wattron(window, A_REVERSE);
  TermInt::printStr(window, headrow, headcol + 9, ms.getElement(0)->getContentText());
  if (highlight) wattroff(window, A_REVERSE);
  for (unsigned int i = 0; i < ms.size(); i++) {
    MenuSelectOptionContainer * msoc = ms.getSectionContainer(i);
    for (unsigned int j = 0; j < 3; j++) {
      highlight = ((i * 3) + 1 + j) == selected;
      if (highlight) wattron(window, A_REVERSE);
      TermInt::printStr(window, headrow + 1 + i, headcol + (j * 10), msoc->getOption(j)->getContentText());
      if (highlight) wattroff(window, A_REVERSE);
    }
  }
}

void EditSiteScreen::update() {
  if (defocusedarea != NULL) {
    if (defocusedarea == &mso) {
      MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
      TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
      TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
    }
    else if (defocusedarea == &ms) {
      int headrow = ms.getHeaderRow();
      int headcol = ms.getHeaderCol();
      int lastsel = ms.getLastSelectionPointer();
      if (lastsel == 0) {
        TermInt::printStr(window, headrow, headcol + 9, ms.getElement(0)->getContentText());
      }
      else {
        MenuSelectOptionContainer * msoc = ms.getSectionContainer((lastsel - 1) / 3);
        int internalid = (lastsel - 1) % 3;
        int add = 0;
        if (internalid == 1) add = 10;
        else if (internalid == 2) add = 20;
        TermInt::printStr(window, headrow + 1 + ((lastsel - 1) / 3), headcol + add, msoc->getOption(internalid)->getContentText());
      }
    }
    defocusedarea = NULL;
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
    if (active && msoe->cursorPosition() >= 0) {
      curs_set(1);
      TermInt::moveCursor(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
    }
    else {
      curs_set(0);
    }
  }
  else if (focusedarea == &ms) {
    if (ms.needsRedraw()) {
      redraw();
      return;
    }
    int headrow = ms.getHeaderRow();
    int headcol = ms.getHeaderCol();
    int lastsel = ms.getLastSelectionPointer();
    int sel = ms.getSelectionPointer();
    if (lastsel == 0) {
      TermInt::printStr(window, headrow, headcol + 9, ms.getElement(0)->getContentText());
    }
    else {
      MenuSelectOptionContainer * msoc = ms.getSectionContainer((lastsel - 1) / 3);
      int internalid = (lastsel - 1) % 3;
      int add = 0;
      if (internalid == 1) add = 10;
      else if (internalid == 2) add = 20;
      TermInt::printStr(window, headrow + 1 + ((lastsel - 1) / 3), headcol + add, msoc->getOption(internalid)->getContentText());
    }
    if (sel == 0) {
      wattron(window, A_REVERSE);
      TermInt::printStr(window, headrow, headcol + 9, ms.getElement(0)->getContentText());
      wattroff(window, A_REVERSE);
    }
    else {
      MenuSelectOptionContainer * msoc = ms.getSectionContainer((sel - 1) / 3);
      int internalid = (sel - 1) % 3;
      int add = 0;
      if (internalid == 1) add = 10;
      else if (internalid == 2) add = 20;
      wattron(window, A_REVERSE);
      TermInt::printStr(window, headrow + 1 + ((sel - 1) / 3), headcol + add, msoc->getOption(internalid)->getContentText());
      wattroff(window, A_REVERSE);
      int cursorpos = msoc->getOption(internalid)->cursorPosition();
      if (active && cursorpos >= 0) {
        curs_set(1);
        TermInt::moveCursor(window, headrow + 1 + ((sel - 1) / 3), headcol + add + cursorpos);
      }
      else {
        curs_set(0);
      }
    }
  }
}

void EditSiteScreen::keyPressed(unsigned int ch) {
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
          focusedarea = &ms;
          focusedarea->enterFocusFrom(0);
        }
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_LEFT:
      if (focusedarea->goLeft()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_RIGHT:
      if (focusedarea->goRight()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        uicommunicator->newCommand("update");
      }
      break;
    case 10:
      activation = focusedarea->activateSelected();
      if (!activation) {
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 'd':
      if (operation == "add") {
        site = new Site();
      }
      for(unsigned int i = 0; i < mso.size(); i++) {
        MenuSelectOptionElement * msoe = mso.getElement(i);
        std::string identifier = msoe->getIdentifier();
        if (identifier == "name") {
            site->setName(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "addr") {
            site->setAddress(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "port") {
            site->setPort(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "user") {
            site->setUser(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "pass") {
            site->setPass(((MenuSelectOptionTextField *)msoe)->getData());
        }
        else if (identifier == "logins") {
            site->setMaxLogins(((MenuSelectOptionNumArrow *)msoe)->getData());
        }
        else if (identifier == "maxup") {
            site->setMaxUp(((MenuSelectOptionNumArrow *)msoe)->getData());
        }
        else if (identifier == "maxdn") {
            site->setMaxDn(((MenuSelectOptionNumArrow *)msoe)->getData());
        }
        else if (identifier == "pret") {
            site->setPRET(((MenuSelectOptionCheckBox *)msoe)->getData());
        }
        else if (identifier == "brokenpasv") {
            site->setBrokenPASV(((MenuSelectOptionCheckBox *)msoe)->getData());
        }
      }
      site->clearSections();
      for (unsigned int i = 0; i < ms.size(); i++) {
        MenuSelectOptionContainer * msoc = ms.getSectionContainer(i);
        std::string name = ((MenuSelectOptionTextField *)msoc->getOption(0))->getData();
        std::string path = ((MenuSelectOptionTextField *)msoc->getOption(1))->getData();
        if (name.length() > 0 && path.length() > 0) {
          site->addSection(name, path);
        }
      }
      if (operation == "add") {
        global->getSiteManager()->addSite(site);
      }
      global->getSiteThreadManager()->getSiteThread(site->getName())->setNumConnections(site->getMaxLogins());
      uicommunicator->newCommand("return");
      return;
    case 'c':
      uicommunicator->newCommand("return");
      break;
  }
}

std::string EditSiteScreen::getLegendText() {
  return currentlegendtext;
}
