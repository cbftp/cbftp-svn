#include "mainscreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../race.h"
#include "../../engine.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../sitelogicmanager.h"

#include "../menuselectoptioncheckbox.h"
#include "../uicommunicator.h"
#include "../termint.h"
#include "../focusablearea.h"
#include "../menuselectsiteelement.h"

extern GlobalContext * global;

MainScreen::MainScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  msslegendtext = "[Enter] Details - [Down] Next option - [Up] Previous option - [b]rowse site - ra[w] command - [A]dd site - [E]dit site - [C]opy site - [D]elete site - [G]lobal settings - Event [l]og";
  msolegendtext = "[Enter] Details - [Down] Next option - [Up] Previous option - [G]lobal settings - Event [l]og";
  this->uicommunicator = uicommunicator;
  mso.makeLeavableDown();
  autoupdate = true;
  currentviewspan = 0;
  sitestartrow = 0;
  currentraces = 0;
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
  bool listraces = global->getEngine()->allRaces();
  unsigned int irow = 1;
  if (listraces) {
    mss.makeLeavableUp();
    mso.clear();
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
  unsigned int position = mss.getSelectionPointer();
  sitestartrow = irow + 4;
  unsigned int pagerows = (unsigned int) (row - sitestartrow) / 2;
  if (position < currentviewspan || position >= currentviewspan + row - sitestartrow) {
    if (position < pagerows) {
      currentviewspan = 0;
    }
    else {
      currentviewspan = position - pagerows;
    }
  }
  if (currentviewspan + row >= mss.size() && mss.size() + 1 >= row - sitestartrow) {
    currentviewspan = mss.size() + 1 - row + sitestartrow;
    if (currentviewspan > position) {
      currentviewspan = position;
    }
  }
  update();
}

void MainScreen::update() {
  if (uicommunicator->hasNewCommand()) {
    if (uicommunicator->getCommand() == "yes") {
      global->getSiteLogicManager()->deleteSiteLogic(deletesite);
      global->getSiteManager()->deleteSite(deletesite);
    }
    uicommunicator->checkoutCommand();
    redraw();
    uicommunicator->newCommand("update");
  }
  else {
    int newcurrentraces = global->getEngine()->currentRaces();
    if (newcurrentraces != currentraces) {
      currentraces = newcurrentraces;
      redraw();
      return;
    }
    currentraces = newcurrentraces;
    if (currentraces) {
      TermInt::printStr(window, 1, 1, "Active races: " + global->int2Str(currentraces));
    }
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
    for (unsigned int i = 0; i + currentviewspan < mss.size() && i < row - sitestartrow; i++) {
      unsigned int listi = i + currentviewspan;
      highlight = false;
      if (mss.isFocused() && selected == listi) {
        highlight = true;
      }
      if (highlight) wattron(window, A_REVERSE);
      TermInt::printStr(window, i + sitestartrow, 1, mss.getSiteLine(listi));
      if (highlight) wattroff(window, A_REVERSE);
    }
  }
}

void MainScreen::keyPressed(unsigned int ch) {
  Site * site;
  std::string target;
  bool update = false;
  unsigned int pagerows = (unsigned int) (row - sitestartrow) * 0.6;
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
          if (mss.getSelectionPointer() < currentviewspan) {
            uicommunicator->newCommand("redraw");
          }
          else {
            uicommunicator->newCommand("update");
          }
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
          if (mss.getSelectionPointer() >= currentviewspan + row - sitestartrow) {
            uicommunicator->newCommand("redraw");
          }
          else {
            uicommunicator->newCommand("update");
          }
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
    case 'G':
      uicommunicator->newCommand("globaloptions");
      break;
    case 'l':
      uicommunicator->newCommand("eventlog");
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
      case 'w':
        if (mss.getSite() == NULL) break;
        uicommunicator->newCommand("rawcommand", mss.getSite()->getName());
        break;
      case KEY_NPAGE:
        for (unsigned int i = 0; i < pagerows; i++) {
          if (!mss.goDown()) {
            break;
          }
          else if (!update) {
            update = true;
          }
          if (!focusedarea->isFocused()) {
            focusedarea->enterFocusFrom(2);
          }
        }
        if (mss.getSelectionPointer() >= currentviewspan + row - sitestartrow) {
          uicommunicator->newCommand("redraw");
        }
        else if (update) {
          uicommunicator->newCommand("update");
        }
        break;
      case KEY_PPAGE:
        for (unsigned int i = 0; i < pagerows; i++) {
          if (!mss.goUp()) {
            break;
          }
          else if (!update) {
            update = true;
          }
          if (!focusedarea->isFocused()) {
            focusedarea->enterFocusFrom(0);
          }
        }
        if (mss.getSelectionPointer() < currentviewspan) {
          uicommunicator->newCommand("redraw");
        }
        else if (update) {
          uicommunicator->newCommand("update");
        }
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

std::string MainScreen::getInfoLabel() {
  return "CLUSTERBOMB MAIN";
}
