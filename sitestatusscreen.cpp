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
  mvwprintw(window, 1, 1, std::string("Detailed status for " + site->getName()).c_str());
  SiteThread * st = global->getSiteThreadManager()->getSiteThread(site->getName());
  mvwprintw(window, 3, 1, std::string("Login slots:    " + global->int2Str(st->getCurrLogins()) + "/" + global->int2Str(site->getMaxLogins())).c_str());
  mvwprintw(window, 4, 1, std::string("Upload slots:   " + global->int2Str(st->getCurrUp()) + "/" + global->int2Str(site->getMaxUp())).c_str());
  mvwprintw(window, 5, 1, std::string("Download slots: " + global->int2Str(st->getCurrDown()) + "/" + global->int2Str(site->getMaxDown())).c_str());
  mvwprintw(window, 7, 1, "Login threads:");
  int i = 8;
  std::vector<FTPThread *>::iterator it2;
  for(it2 = st->getConns()->begin(); it2 != st->getConns()->end(); it2++) {
    int id = (*it2)->getId();
    std::string status = (*it2)->getStatus();
    mvwprintw(window, i++, 1, std::string("#" + global->int2Str(id) + " - " + status).c_str());
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
