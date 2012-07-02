#include "editsitescreen.h"

EditSiteScreen::EditSiteScreen(WINDOW * window, UIWindowCommand * windowcommand, int row, int col) {
  this->windowcommand = windowcommand;
  active = false;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one, save changes - [c]ancel, undo changes";
  currentlegendtext = defaultlegendtext;
  operation = windowcommand->getArg1();
  std::string arg2 = windowcommand->getArg2();
  windowcommand->checkoutCommand();
  if (operation == "add") {
    modsite = Site("SUNET");
  }
  else if (operation == "edit") {
    site = global->getSiteManager()->getSite(arg2);
    modsite = Site(*site);
  }
  int y = 3;
  int x = 1;

  mso.addStringField(y++, x, "name", "Name:", modsite.getName(), false);
  mso.addStringField(y++, x, "addr", "Address:", modsite.getAddress(), false);
  mso.addStringField(y++, x, "port", "Port:", modsite.getPort(), false);
  mso.addStringField(y++, x, "user", "Username:", modsite.getUser(), false);
  mso.addStringField(y++, x, "pass", "Password:", modsite.getPass(), true);
  mso.addIntArrow(y++, x, "logins", "Login slots:", modsite.getMaxLogins(), 0, 99);
  mso.addIntArrow(y++, x, "maxup", "Upload slots:", modsite.getMaxUp(), 0, 99);
  mso.addIntArrow(y++, x, "maxdn", "Download slots:", modsite.getMaxDown(), 0, 99);
  mso.addCheckBox(y++, x, "pret", "Needs PRET:", modsite.needsPRET());
  mso.addCheckBox(y++, x, "brokenpasv", "Broken PASV:", modsite.hasBrokenPASV());
  init(window, row, col);
}

void EditSiteScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "-== SITE OPTIONS ==-");
  bool highlight;
  for (int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    if (highlight) wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void EditSiteScreen::update() {
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

void EditSiteScreen::keyPressed(int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      windowcommand->newCommand("updatesetlegend");
      return;
    }
    activeelement->inputChar(ch);
    windowcommand->newCommand("update");
    return;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      mso.goPrev();
      windowcommand->newCommand("update");
      break;
    case KEY_DOWN:
      mso.goNext();
      windowcommand->newCommand("update");
      break;
    case 10:
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      if (!activation) {
        windowcommand->newCommand("update");
        break;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      windowcommand->newCommand("updatesetlegend");
      break;
    case 'd':
      if (operation == "add") {
        site = new Site();
      }
      for(int i = 0; i < mso.size(); i++) {
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
      if (operation == "add") {
        global->getSiteManager()->addSite(site);
      }
      windowcommand->newCommand("return");
      return;
    case 'c':
      windowcommand->newCommand("return");
      break;
  }
}

std::string EditSiteScreen::getLegendText() {
  return currentlegendtext;
}
