#include "mainscreen.h"

MainScreen::MainScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
	mss.setWindow(window);
  this->uicommunicator = uicommunicator;
  init(window, row, col);
}

void MainScreen::redraw() {
  werase(window);
  curs_set(0);
  TermInt::printStr(window, 1, 1, "-=== MAIN SCREEN ===-");
  TermInt::printStr(window, 3, 1, "Sites added: " + global->int2Str(global->getSiteManager()->getNumSites()));
  if (global->getSiteManager()->getNumSites()) {
    TermInt::printStr(window, 5, 1, "Site    Logins  Uploads  Downloads");
  }
  else {
    TermInt::printStr(window, 5, 1, "Press 'A' to add a site");
  }
  int x = 1;
  int y = 7;
  mss.prepareRefill();
  for (std::vector<Site *>::iterator it = global->getSiteManager()->getSitesIteratorBegin(); it != global->getSiteManager()->getSitesIteratorEnd(); it++) {
    mss.add(*it, y++, x);
  }
  mss.print();
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
}

void MainScreen::keyPressed(unsigned int ch) {
  Site * site;
  switch(ch) {
    case KEY_UP:
      if (mss.getSite() == NULL) break;
      mss.goPrev();
      uicommunicator->newCommand("update");
      break;
    case KEY_DOWN:
      if (mss.getSite() == NULL) break;
      mss.goNext();
      uicommunicator->newCommand("update");
      break;
    case ' ':
    case 10:
      if (mss.getSite() == NULL) break;
      uicommunicator->newCommand("sitestatus", mss.getSite()->getName());
      break;
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

std::string MainScreen::getLegendText() {
  return "[Enter] Details - [Down] Next option - [Up] Previous option - [b]rowse site - [E]dit site - [C]opy site - [D]elete site";
}
