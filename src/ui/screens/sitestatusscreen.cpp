#include "sitestatusscreen.h"

SiteStatusScreen::SiteStatusScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  sitename = uicommunicator->getArg1();
  uicommunicator->checkoutCommand();
  site = global->getSiteManager()->getSite(sitename);
  autoupdate = true;
  st = global->getSiteLogicManager()->getSiteLogic(site->getName());
  for(unsigned int j = 0; j < st->getConns()->size(); j++) {
    previousstatuslength.push_back(0);
  }
  init(window, row, col);
}

void SiteStatusScreen::redraw() {
  werase(window);
  curs_set(0);
  update();
}

void SiteStatusScreen::update() {
  std::string loginslots = "Login slots:    " + global->int2Str(st->getCurrLogins());
  if (!site->unlimitedLogins()) {
    loginslots += "/" + global->int2Str(site->getMaxLogins());
  }
  std::string upslots = "Upload slots:   " + global->int2Str(st->getCurrUp());
  if (!site->unlimitedUp()) {
    upslots += "/" + global->int2Str(site->getMaxUp());
  }
  std::string downslots = "Download slots: " + global->int2Str(st->getCurrDown());
  if (!site->unlimitedDown()) {
    downslots += "/" + global->int2Str(site->getMaxDown());
  }
  TermInt::printStr(window, 1, 1, loginslots);
  TermInt::printStr(window, 2, 1, upslots);
  TermInt::printStr(window, 3, 1, downslots);
  TermInt::printStr(window, 5, 1, "Login threads:");
  int i = 8;
  for(unsigned int j = 0; j < st->getConns()->size(); j++) {
    std::string status = st->getStatus(j);
    int statuslength = status.length();
    while (status.length() < previousstatuslength[j]) {
      status += " ";
    }
    previousstatuslength[j] = statuslength;
    TermInt::printStr(window, i++, 1, "#" + global->int2Str((int)j) + " - " + status);
  }
}

void SiteStatusScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case KEY_RIGHT:
      uicommunicator->newCommand("rawdata", site->getName(), "0");
      break;
    case 'E':
      uicommunicator->newCommand("editsite", "edit", site->getName());
      break;
    case ' ':
    case 10:
      uicommunicator->newCommand("return");
      break;
    case 'b':
      uicommunicator->newCommand("browse", site->getName());
      break;
    case 'w':
      uicommunicator->newCommand("rawcommand", site->getName());
      break;
  }
}

std::string SiteStatusScreen::getLegendText() {
  return "[Right] Raw data screens - [Enter] Return - ra[w] command - [E]dit site";
}

std::string SiteStatusScreen::getInfoLabel() {
  return "DETAILED STATUS: " + site->getName();
}
