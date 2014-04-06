#include "newracescreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../engine.h"

#include "../uicommunicator.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptionelement.h"
#include "../focusablearea.h"
#include "../termint.h"

extern GlobalContext * global;

NewRaceScreen::NewRaceScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [t]oggle all - [s]tart race - [S]tart race and return to browsing - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  toggleall = false;
  unsigned int y = 2;
  unsigned int x = 1;
  sectionupdate = false;
  infotext = "";
  std::string sectionstring = uicommunicator->getArg2();
  size_t splitpos;
  bool sectionset = false;
  int sectx = x + std::string("Section: ").length();
  while (sectionstring.length() > 0) {
    splitpos = sectionstring.find(";");
    std::string section;
    if (splitpos != std::string::npos) {
      section = sectionstring.substr(0, splitpos);
      sectionstring = sectionstring.substr(splitpos + 1);
    }
    else {
      section = sectionstring;
      sectionstring = "";
    }
    std::string buttontext = " " + section + " ";
    if (!sectionset) {
      this->section = section;
      sectionset = true;
    }
    msos.addTextButton(y, sectx, section, buttontext);
    sectx = sectx + buttontext.length();
  }
  y = y + 2;
  release = uicommunicator->getArg3();
  startsite = global->getSiteManager()->getSite(uicommunicator->getArg1());
  focusedarea = &msos;
  msos.makeLeavableDown();
  mso.makeLeavableUp();
  msos.enterFocusFrom(0);
  populateSiteList();
  init(window, row, col);
}

void NewRaceScreen::populateSiteList() {
  int y = 6;
  int x = 1;
  std::vector<Site *>::iterator it;
  mso.clear();
  for (it = global->getSiteManager()->getSitesIteratorBegin(); it != global->getSiteManager()->getSitesIteratorEnd(); it++) {
    Site * site = *it;
    if (site->hasSection(section)) {
      mso.addCheckBox(y++, x, site->getName(), site->getName(), toggleall || site == startsite);
    }
  }
}
void NewRaceScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "Release: " + release);
  TermInt::printStr(window, 2, 1, "Section: ");
  bool highlight;
  for (unsigned int i = 0; i < msos.size(); i++) {
    MenuSelectOptionElement * msoe = msos.getElement(i);
    highlight = false;
    if (msos.isFocused() && msos.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe));
    if (highlight) wattroff(window, A_REVERSE);
  }
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
    if (highlight) wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getContentText());
  }
}

void NewRaceScreen::update() {
  if (sectionupdate) {
    sectionupdate = false;
    populateSiteList();
    redraw();
    return;
  }
  if (defocusedarea != NULL) {
    if (defocusedarea == &mso) {
      MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
      TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
      TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getContentText());
    }
    else if (defocusedarea == &msos){
      MenuSelectOptionElement * msoe = msos.getElement(msos.getLastSelectionPointer());
      TermInt::printStr(window, msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe));
    }
  }
  if (focusedarea == &mso) {
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
  else {
    MenuSelectOptionElement * msoe = msos.getElement(msos.getLastSelectionPointer());
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe));
    msoe = msos.getElement(msos.getSelectionPointer());
    wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe));
    wattroff(window, A_REVERSE);
  }
}

void NewRaceScreen::keyPressed(unsigned int ch) {
  infotext = "";
  unsigned int pagerows = (unsigned int) (row - 6) * 0.6;
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
          focusedarea = &msos;
          focusedarea->enterFocusFrom(2);
        }
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_DOWN:
      if (focusedarea->goDown()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mso;
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
    case KEY_NPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (focusedarea->goDown()) {
          if (!focusedarea->isFocused()) {
            defocusedarea = focusedarea;
            focusedarea = &mso;
            focusedarea->enterFocusFrom(0);
          }
        }
      }
      uicommunicator->newCommand("redraw");
      break;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (focusedarea->goUp()) {
          if (!focusedarea->isFocused()) {
            defocusedarea = focusedarea;
            focusedarea = &msos;
            focusedarea->enterFocusFrom(2);
          }
        }
      }
      uicommunicator->newCommand("redraw");
      break;
    case 10:

      activation = focusedarea->getElement(focusedarea->getSelectionPointer())->activate();
      if (!activation) {
        if (focusedarea == &msos) {
          section = msos.getElement(msos.getSelectionPointer())->getIdentifier();
          sectionupdate = true;
        }
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 27: // esc
    case 'c':
      uicommunicator->newCommand("return");
      break;
    case 's':
      if (startRace()) {
        uicommunicator->newCommand("returnracestatus", release);
      }
      break;
    case 'S':
      if (startRace()) {
        uicommunicator->newCommand("return");
      }
      break;
    case 't':
      if (!toggleall) {
        toggleall = true;
      }
      else {
        toggleall = false;
      }
      populateSiteList();
      uicommunicator->newCommand("redraw");
      break;
  }
}

std::string NewRaceScreen::getLegendText() {
  return currentlegendtext;
}

std::string NewRaceScreen::getInfoLabel() {
  return "START NEW RACE";
}

std::string NewRaceScreen::getInfoText() {
  return infotext;
}

std::string NewRaceScreen::getSectionButtonText(MenuSelectOptionElement * msoe) {
  std::string buttontext = msoe->getLabelText();
  if (msoe->getIdentifier() == section) {
    buttontext[0] = '[';
    buttontext[buttontext.length()-1] = ']';
  }
  return buttontext;
}

bool NewRaceScreen::startRace() {
  std::list<std::string> sites;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionCheckBox * msocb = (MenuSelectOptionCheckBox *) mso.getElement(i);
    if (msocb->getData()) {
      sites.push_back(msocb->getIdentifier());
    }
  }
  if (sites.size() < 2) {
    infotext = "Cannot start race with less than 2 sites!";
    uicommunicator->newCommand("update");
    return false;
  }
  global->getEngine()->newRace(release, section, sites);
  return true;
}
