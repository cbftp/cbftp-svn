#include "sitestatusscreen.h"

#include "../../sitemanager.h"
#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../ftpconn.h"
#include "../../connstatetracker.h"
#include "../../util.h"

#include "../ui.h"

extern GlobalContext * global;

SiteStatusScreen::SiteStatusScreen(Ui * ui) {
  this->ui = ui;
}

void SiteStatusScreen::initialize(unsigned int row, unsigned int col, std::string sitename) {
  this->sitename = sitename;
  site = global->getSiteManager()->getSite(sitename);
  autoupdate = true;
  st = global->getSiteLogicManager()->getSiteLogic(site->getName());
  for(unsigned int j = 0; j < st->getConns()->size(); j++) {
    previousstatuslength.push_back(0);
  }
  init(row, col);
}

void SiteStatusScreen::redraw() {
  ui->erase();
  ui->hideCursor();
  update();
}

void SiteStatusScreen::update() {
  std::string loginslots = "Login slots:    " + util::int2Str(st->getCurrLogins());
  if (!site->unlimitedLogins()) {
    loginslots += "/" + util::int2Str(site->getMaxLogins());
  }
  std::string upslots = "Upload slots:   " + util::int2Str(st->getCurrUp());
  if (!site->unlimitedUp()) {
    upslots += "/" + util::int2Str(site->getMaxUp());
  }
  std::string downslots = "Download slots: " + util::int2Str(st->getCurrDown());
  if (!site->unlimitedDown()) {
    downslots += "/" + util::int2Str(site->getMaxDown());
  }
  ui->printStr(1, 1, loginslots);
  ui->printStr(2, 1, upslots);
  ui->printStr(3, 1, downslots);
  ui->printStr(5, 1, "Login threads:");
  int i = 8;
  for(unsigned int j = 0; j < st->getConns()->size(); j++) {
    std::string status = st->getStatus(j);
    std::string llstate = st->getConnStateTracker(j)->isListLocked()
        ? "Y" : "N";
    std::string hlstate = st->getConnStateTracker(j)->isHardLocked()
        ? "Y" : "N";
    std::string tstate = st->getConnStateTracker(j)->hasFileTransfer()
        ? "Y" : "N";
    int statuslength = status.length();
    while (status.length() < previousstatuslength[j]) {
      status += " ";
    }
    previousstatuslength[j] = statuslength;
    ui->printStr(i++, 1, "#" + util::int2Str((int)j) + " - LL:" + llstate +
        " - HL:" + hlstate + " - T:" + tstate + " - " + status);
  }
}

bool SiteStatusScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case KEY_RIGHT:
      ui->goRawData(site->getName());
      return true;
    case 'E':
      ui->goEditSite(site->getName());
      return true;
    case 27: // esc
    case ' ':
    case 10:
      ui->returnToLast();
      return true;
    case 'b':
      ui->goBrowse(site->getName());
      return true;
    case 'w':
      ui->goRawCommand(site->getName());
      return true;
  }
  return false;
}

std::string SiteStatusScreen::getLegendText() const {
  return "[Right] Raw data screens - [Enter] Return - ra[w] command - [E]dit site";
}

std::string SiteStatusScreen::getInfoLabel() const {
  return "DETAILED STATUS: " + site->getName();
}
