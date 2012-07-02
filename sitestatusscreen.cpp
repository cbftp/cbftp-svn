#include "sitestatusscreen.h"

SiteStatusScreen::SiteStatusScreen(WINDOW * window, UIWindowCommand * windowcommand, int row, int col) {
  this->windowcommand = windowcommand;
  sitename = windowcommand->getArg1();
  windowcommand->checkoutCommand();
  site = global->getSiteManager()->getSite(sitename);
  init(window, row, col);
}

void SiteStatusScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "Detailed status for " + site->getName());
  SiteThread * st = global->getSiteThreadManager()->getSiteThread(site->getName());
  TermInt::printStr(window, 3, 1, "Login slots:    " + global->int2Str(st->getCurrLogins()) + "/" + global->int2Str(site->getMaxLogins()));
  TermInt::printStr(window, 4, 1, "Upload slots:   " + global->int2Str(st->getCurrUp()) + "/" + global->int2Str(site->getMaxUp()));
  TermInt::printStr(window, 5, 1, "Download slots: " + global->int2Str(st->getCurrDown()) + "/" + global->int2Str(site->getMaxDown()));
  TermInt::printStr(window, 7, 1, "Login threads:");
  int i = 8;
  std::vector<FTPThread *>::iterator it2;
  for(it2 = st->getConns()->begin(); it2 != st->getConns()->end(); it2++) {
    int id = (*it2)->getId();
    std::string status = (*it2)->getStatus();
    TermInt::printStr(window, i++, 1, "#" + global->int2Str(id) + " - " + status);
  }
}

void SiteStatusScreen::update() {

}

void SiteStatusScreen::keyPressed(int ch) {
  switch(ch) {
    case KEY_RIGHT:
      windowcommand->newCommand("rawdata", sitename, "0");
      break;
    case 'E':
      windowcommand->newCommand("editsite", "edit", sitename);
      break;
    case ' ':
    case 10:
      windowcommand->newCommand("return");
      break;
  }
}

std::string SiteStatusScreen::getLegendText() {
  return "[Right] Raw data screens - [Enter] Return - [E]dit site";
}
