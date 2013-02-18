#include "mainscreen.h"

MainScreen::MainScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  msslegendtext = "[Enter] Details - [Down] Next option - [Up] Previous option - [b]rowse site - [A]dd site - [E]dit site - [C]opy site - [D]elete site";
  msolegendtext = "[Enter] Details - [Down] Next option - [Up] Previous option";
  this->uicommunicator = uicommunicator;
  mso.makeLeavableDown();
  autoupdate = true;
  if (global->getEngine()->currentRaces()) {
    focusedarea = &mso;
    mso.enterFocusFrom(0);
    mss.makeLeavableUp();
  }
  else {
    mss.enterFocusFrom(0);
    focusedarea = &mss;
  }
  init(window, row, col);
}

void MainScreen::redraw() {
  werase(window);
  curs_set(0);
  TermInt::printStr(window, 1, 1, "-=== MAIN SCREEN ===-");
  bool listraces = global->getEngine()->allRaces();
  unsigned int irow = 3;
  if (listraces) {
    mss.makeLeavableUp();
    mso.clear();
    TermInt::printStr(window, irow++, 1, "Active races: " + global->int2Str(global->getEngine()->currentRaces()));
    irow++;
    TermInt::printStr(window, irow++, 1, "Section  Name");
    std::list<Race *>::iterator it;
    for (it = global->getEngine()->getRacesIteratorBegin(); it != global->getEngine()->getRacesIteratorEnd(); it++) {
      std::string release = (*it)->getName();
      mso.addCheckBox(irow++, 1, release, release, false);
    }
    irow++;
  }
  TermInt::printStr(window, irow, 1, "Sites added: " + global->int2Str(global->getSiteManager()->getNumSites()));
  if (global->getSiteManager()->getNumSites()) {
    TermInt::printStr(window, irow+2, 1, "Site    Logins  Uploads  Downloads");
  }
  else {
    TermInt::printStr(window, irow+2, 1, "Press 'A' to add a site");
  }
  int x = 1;
  int y = irow+4;
  mss.prepareRefill();
  for (std::vector<Site *>::iterator it = global->getSiteManager()->getSitesIteratorBegin(); it != global->getSiteManager()->getSitesIteratorEnd(); it++) {
    mss.add(*it, y++, x);
  }
  mss.checkPointer();
  update();
}

void MainScreen::update() {
  if (uicommunicator->hasNewCommand()) {
    if (uicommunicator->getCommand() == "yes") {
      global->getSiteThreadManager()->deleteSiteThread(deletesite);
      global->getSiteManager()->deleteSite(deletesite);
    }
    uicommunicator->checkoutCommand();
    redraw();
    uicommunicator->newCommand("update");
  }
  else {
    bool highlight;
    unsigned int selected = mso.getSelectionPointer();
    for (unsigned int i = 0; i < mso.size(); i++) {
      highlight = false;
      MenuSelectOptionElement * msoe = mso.getElement(i);
      Race * race = global->getEngine()->getRace(msoe->getIdentifier());
      if (mso.isFocused() && selected == i) {
        highlight = true;
      }
      TermInt::printStr(window, msoe->getRow(), msoe->getCol(), race->getSection());
      if (highlight) wattron(window, A_REVERSE);
      TermInt::printStr(window, msoe->getRow(), msoe->getCol() + 9, msoe->getLabelText());
      if (highlight) wattroff(window, A_REVERSE);
    }
    selected = mss.getSelectionPointer();
    for (unsigned int i = 0; i < mss.size(); i++) {
      highlight = false;
      MenuSelectSiteElement * msse = mss.getSiteElement(i);
      if (mss.isFocused() && selected == i) {
        highlight = true;
      }
      if (highlight) wattron(window, A_REVERSE);
      TermInt::printStr(window, msse->getRow(), msse->getCol(), mss.getSiteLine(i));
      if (highlight) wattroff(window, A_REVERSE);
    }
  }
}

void MainScreen::keyPressed(unsigned int ch) {
  Site * site;
  std::string target;
  switch(ch) {
    case KEY_UP:
      if (focusedarea->goUp()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mso;
          focusedarea->enterFocusFrom(2);
          uicommunicator->newCommand("updatesetlegend");
        }
        else {
          uicommunicator->newCommand("update");
        }
      }
      break;
    case KEY_DOWN:
      if (focusedarea->goDown()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mss;
          focusedarea->enterFocusFrom(0);
          uicommunicator->newCommand("updatesetlegend");
        }
        else {
          uicommunicator->newCommand("update");
        }
      }
      break;
    case ' ':
    case 10:
      if (mss.isFocused()) {
        if (mss.getSite() == NULL) break;
        uicommunicator->newCommand("sitestatus", mss.getSite()->getName());
      }
      else if (mso.isFocused() && mso.size() > 0) {
        target = mso.getElement(mso.getSelectionPointer())->getIdentifier();
        uicommunicator->newCommand("racestatus", target);
      }
      break;
  }
  if (mss.isFocused()) {
    switch(ch) {
      case 'E':
        if (mss.getSite() == NULL) break;
        uicommunicator->newCommand("editsite", "edit", mss.getSite()->getName());
        break;
      case 'A':
        uicommunicator->newCommand("editsite", "add");
        break;
      case 'C':
        if (mss.getSite() == NULL) break;
        site = new Site(*mss.getSite());
        int i;
        for (i = 0; global->getSiteManager()->getSite(site->getName() + "-" + global->int2Str(i)) != NULL; i++);
        site->setName(site->getName() + "-" + global->int2Str(i));
        global->getSiteManager()->addSite(site);
        uicommunicator->newCommand("redraw");
        break;
      case 'D':
        site = mss.getSite();
        if (site == NULL) break;
        deletesite = site->getName();
        uicommunicator->newCommand("confirmation");
        break;
      case 'b':
        if (mss.getSite() == NULL) break;
        uicommunicator->newCommand("browse", mss.getSite()->getName());
        break;
    }
  }
}

std::string MainScreen::getLegendText() {
  if (focusedarea == &mss) {
    return msslegendtext;
  }
  return msolegendtext;
}
