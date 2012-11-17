#include "newracescreen.h"

NewRaceScreen::NewRaceScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [t]oggle all - [s]tart race - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  toggleall = true;
  unsigned int y = 6;
  unsigned int x = 1;
  section = uicommunicator->getArg2();
  release = uicommunicator->getArg3();
  modsite = global->getSiteManager()->getSite(uicommunicator->getArg1());
  std::vector<Site *>::iterator it;
  for (it = global->getSiteManager()->getSitesIteratorBegin(); it != global->getSiteManager()->getSitesIteratorEnd(); it++) {
    Site * site = *it;
    if (site->hasSection(section)) {
      mso.addCheckBox(y++, x, site->getName(), site->getName(), true);
    }
  }
  init(window, row, col);
}

void NewRaceScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "-== NEW RACE ==-");
  TermInt::printStr(window, 3, 1, "Release: " + release);
  TermInt::printStr(window, 4, 1, "Section: " + section);
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
    if (highlight) wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getContentText());
  }
}

void NewRaceScreen::update() {
  MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  wattron(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
  wattroff(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getContentText());
  if (active && msoe->cursorPosition() >= 0) {
    curs_set(1);
    TermInt::moveCursor(window, msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1 + msoe->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void NewRaceScreen::keyPressed(unsigned int ch) {
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
  std::list<std::string> sites;
  switch(ch) {
    case KEY_UP:
      mso.goUp();
      uicommunicator->newCommand("update");
      break;
    case KEY_DOWN:
      mso.goDown();
      uicommunicator->newCommand("update");
      break;
    case 10:

      activation = mso.getElement(mso.getSelectionPointer())->activate();
      if (!activation) {
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 'c':
      uicommunicator->newCommand("return");
      break;
    case 's':
      for (unsigned int i = 0; i < mso.size(); i++) {
        MenuSelectOptionCheckBox * msocb = (MenuSelectOptionCheckBox *) mso.getElement(i);
        if (msocb->getData()) {
          sites.push_back(msocb->getIdentifier());
        }
      }
      global->getEngine()->newRace(release, section, sites);
      uicommunicator->newCommand("return");
      break;
    case 't':
      for (unsigned int i = 0; i < mso.size(); i++) {
        MenuSelectOptionCheckBox * msocb = (MenuSelectOptionCheckBox *) mso.getElement(i);
        if ((toggleall && msocb->getData()) || (!toggleall && !msocb->getData())) {
          msocb->activate();
        }
      }
      if (!toggleall) {
        toggleall = true;
      }
      else {
        toggleall = false;
      }
      uicommunicator->newCommand("redraw");
      break;
  }
}

std::string NewRaceScreen::getLegendText() {
  return currentlegendtext;
}
