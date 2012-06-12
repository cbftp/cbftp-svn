#include "editsitescreen.h"

EditSiteScreen::EditSiteScreen(WINDOW * window, UIWindowCommand * windowcommand, int row, int col) {
  this->windowcommand = windowcommand;
  active = false;
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
  mvwprintw(window, 1, 1, "-== SITE OPTIONS ==-");
  bool highlight;
  for (int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    mvwprintw(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText().c_str());
    if (highlight) wattroff(window, A_REVERSE);
    mvwprintw(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText().c_str());
  }
}

void EditSiteScreen::update() {
  MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
  mvwprintw(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText().c_str());
  mvwprintw(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText().c_str());
  msoe = mso.getElement(mso.getSelectionPointer());
  wattron(window, A_REVERSE);
  mvwprintw(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText().c_str());
  wattroff(window, A_REVERSE);
  mvwprintw(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText().c_str());
  if (active && msoe->cursorPosition() >= 0) {
    curs_set(1);
    wmove(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void EditSiteScreen::keyPressed(int ch) {
  if (active) {
    windowcommand->newCommand("update");
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      return;
    }
    activeelement->inputChar(ch);
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
      windowcommand->newCommand("update");
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      if (!activation) {
        break;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
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
        else if (identifier == "address") {
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
