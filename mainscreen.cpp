#include "mainscreen.h"

MainScreen::MainScreen(WINDOW * window, UIWindowCommand * windowcommand, int row, int col) {
	mss.setWindow(window);
  this->windowcommand = windowcommand;
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
  if (windowcommand->hasNewCommand()) {
    if (windowcommand->getCommand() == "yes") {
      global->getSiteThreadManager()->deleteSiteThread(deletesite);
      global->getSiteManager()->deleteSite(deletesite);
    }
    windowcommand->checkoutCommand();
    redraw();
    windowcommand->newCommand("update");
  }
}

void MainScreen::keyPressed(int ch) {
  Site * site;
  switch(ch) {
    case KEY_UP:
      if (mss.getSite() == NULL) break;
      mss.goPrev();
      windowcommand->newCommand("update");
      break;
    case KEY_DOWN:
      if (mss.getSite() == NULL) break;
      mss.goNext();
      windowcommand->newCommand("update");
      break;
    case ' ':
    case 10:
      if (mss.getSite() == NULL) break;
      windowcommand->newCommand("sitestatus", mss.getSite()->getName());
      break;
    case 'E':
      if (mss.getSite() == NULL) break;
      windowcommand->newCommand("editsite", "edit", mss.getSite()->getName());
      break;
    case 'A':
      windowcommand->newCommand("editsite", "add");
      break;
    case 'C':
      if (mss.getSite() == NULL) break;
      site = new Site(*mss.getSite());
      int i;
      for (i = 0; global->getSiteManager()->getSite(site->getName() + "-" + global->int2Str(i)) != NULL; i++);
      site->setName(site->getName() + "-" + global->int2Str(i));
      global->getSiteManager()->addSite(site);
      windowcommand->newCommand("redraw");
      break;
    case 'D':
      site = mss.getSite();
      if (site == NULL) break;
      deletesite = site->getName();
      windowcommand->newCommand("confirmation");
      break;
  }
}

std::string MainScreen::getLegendText() {
  return "[Enter] Details - [Down] Next option - [Up] Previous option - [E]dit site - [C]opy site - [D]elete site";
}
