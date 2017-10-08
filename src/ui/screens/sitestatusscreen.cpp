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

SiteStatusScreen::SiteStatusScreen(Ui * ui) {
  this->ui = ui;
}

void SiteStatusScreen::initialize(unsigned int row, unsigned int col, std::string sitename) {
  this->sitename = sitename;
  site = global->getSiteManager()->getSite(sitename);
  autoupdate = true;
  st = global->getSiteLogicManager()->getSiteLogic(site->getName());
  init(row, col);
}

void SiteStatusScreen::redraw() {
  ui->erase();
  ui->hideCursor();
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
    ui->printStr(i++, 1, "#" + util::int2Str((int)j) + " - LL:" + llstate +
        " - HL:" + hlstate + " - T:" + tstate + " - " + status);
  }
  ++i;
  unsigned long long int sizeupday = site->getSizeUpLast24Hours();
  unsigned int filesupday = site->getFilesUpLast24Hours();
  unsigned long long int sizeupall = site->getSizeUpAll();
  unsigned int filesupall = site->getFilesUpAll();
  unsigned long long int sizedownday = site->getSizeDownLast24Hours();
  unsigned int filesdownday = site->getFilesDownLast24Hours();
  unsigned long long int sizedownall = site->getSizeDownAll();
  unsigned int filesdownall = site->getFilesDownAll();
  ui->printStr(i++, 1, "Traffic measurements");
  ui->printStr(i++, 1, "Upload   last 24 hours: " + util::parseSize(sizeupday) + ", " +
                 util::int2Str(filesupday) + " files - All time: " + util::parseSize(sizeupall) + ", " +
                 util::int2Str(filesupall) + " files");
  ui->printStr(i++, 1, "Download last 24 hours: " + util::parseSize(sizedownday) + ", " +
                 util::int2Str(filesdownday) + " files - All time: " + util::parseSize(sizedownall) + ", " +
                 util::int2Str(filesdownall) + " files");
}

void SiteStatusScreen::update() {
  redraw();
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
