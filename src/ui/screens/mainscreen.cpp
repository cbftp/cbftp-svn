#include "mainscreen.h"

#include <cctype>

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../race.h"
#include "../../engine.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../sitelogicmanager.h"
#include "../../transferjob.h"
#include "../../util.h"
#include "../../remotecommandhandler.h"
#include "../../pointer.h"

#include "../menuselectoptioncheckbox.h"
#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectsiteelement.h"

#include "allracesscreen.h"
#include "alltransferjobsscreen.h"

extern GlobalContext * global;

MainScreen::MainScreen(Ui * ui) {
  this->ui = ui;
}

void MainScreen::initialize(unsigned int row, unsigned int col) {
  siteextralegendtext = " - [Tab] split browse - [right/b]rowse site - ra[w] command - [A]dd site - [E]dit site - [C]opy site - [D]elete site - [q]uick jump";
  baselegendtext = "[Enter] Details - [Down] Next option - [Up] Previous option - [G]lobal settings - Event [l]og - [t]ransfers - All [r]aces - All transfer[j]obs - toggle [U]dp - Browse lo[c]al - [Esc] back to browsing";
  gotolegendtext = "[Any] Go to matching first letter in site list - [Esc] Cancel";
  autoupdate = true;
  gotomode = false;
  currentviewspan = 0;
  sitestartrow = 0;
  currentraces = 0;
  currenttransferjobs = 0;
  if (global->getEngine()->currentRaces()) {
    focusedarea = &mso;
    mso.enterFocusFrom(0);
  }
  else if (global->getEngine()->currentTransferJobs()) {
    focusedarea = &msot;
    msot.enterFocusFrom(0);
  }
  else {
    focusedarea = &mss;
    mss.enterFocusFrom(0);
  }
  init(row, col);
}

void MainScreen::redraw() {
  ui->erase();
  ui->hideCursor();
  numsitestext = "Sites: " + util::int2Str(global->getSiteManager()->getNumSites());
  int listraces = global->getEngine()->allRaces();
  int listtransferjobs = global->getEngine()->allTransferJobs();
  unsigned int irow = 0;
  if (listraces) {
    mss.makeLeavableUp();
    msot.makeLeavableUp();
    mso.clear();
    AllRacesScreen::addRaceTableHeader(irow++, mso, std::string("RACE NAME") + (listraces > 3 ? " (Showing latest 3)" : ""));
    std::list<Pointer<Race> >::const_iterator it;
    int i = 0;
    for (it = --global->getEngine()->getRacesEnd(); it != --global->getEngine()->getRacesBegin() && i < 3; it--, i++) {
      AllRacesScreen::addRaceDetails(irow++, mso, *it);
    }
    mso.checkPointer();
    mso.adjustLines(col - 3);
    irow++;
  }
  if (listtransferjobs) {
    mss.makeLeavableUp();
    msot.clear();
    AllTransferJobsScreen::addJobTableHeader(irow++, msot, std::string("TRANSFER JOB NAME") + (listtransferjobs > 3 ? " (Showing latest 3)" : ""));
    std::list<Pointer<TransferJob> >::const_iterator it;
    int i = 0;
    for (it = --global->getEngine()->getTransferJobsEnd(); it != --global->getEngine()->getTransferJobsBegin() && i < 3; it--, i++) {
      AllTransferJobsScreen::addJobDetails(irow++, msot, *it);
    }
    msot.checkPointer();
    msot.adjustLines(col - 3);
    irow++;
  }
  if (global->getSiteManager()->getNumSites()) {
    ui->printStr(irow, 1, "SITE    LOGINS  UPLOADS  DOWNLOADS");
  }
  else {
    ui->printStr(irow, 1, "Press 'A' to add a site");
  }
  int x = 1;
  int y = ++irow;
  mss.prepareRefill();
  for (std::vector<Site *>::const_iterator it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
    mss.add(*it, y++, x);
  }
  mss.checkPointer();
  unsigned int position = mss.getSelectionPointer();
  sitestartrow = irow;
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
  if (listtransferjobs || mss.size()) {
    mso.makeLeavableDown();
  }
  else {
    mso.makeLeavableDown(false);
  }
  if (listraces) {
    msot.makeLeavableUp();
  }
  else {
    msot.makeLeavableUp(false);
  }
  if (mss.size()) {
    msot.makeLeavableDown();
  }
  else {
    msot.makeLeavableDown(false);
  }
  if (listraces || listtransferjobs) {
    mss.makeLeavableUp();
  }
  else {
    mss.makeLeavableUp(false);
  }
  int currentraces = global->getEngine()->currentRaces();
  int currenttransferjobs = global->getEngine()->currentTransferJobs();
  if (currentraces) {
    activeracestext = "Active races: " + util::int2Str(currentraces) + "  ";
  }
  else {
    activeracestext = "";
  }
  if (currenttransferjobs) {
    activejobstext = "Active jobs: " + util::int2Str(currenttransferjobs) + "  ";
  }
  else {
    activejobstext = "";
  }
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<ResizableElement> re = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i && listraces) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
  for (unsigned int i = 0; i < msot.size(); i++) {
    Pointer<ResizableElement> re = msot.getElement(i);
    highlight = false;
    if (msot.isFocused() && msot.getSelectionPointer() == i && listtransferjobs) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
  unsigned int selected = mss.getSelectionPointer();
  for (unsigned int i = 0; i + currentviewspan < mss.size() && i < row - sitestartrow; i++) {
    unsigned int listi = i + currentviewspan;
    highlight = false;
    if (mss.isFocused() && selected == listi) {
      highlight = true;
    }
    ui->printStr(i + sitestartrow, 1, mss.getSiteLine(listi), highlight);
  }

}

void MainScreen::update() {
  redraw();
}

void MainScreen::command(std::string command) {
  if (command == "yes") {
    global->getSiteLogicManager()->deleteSiteLogic(deletesite);
    global->getSiteManager()->deleteSite(deletesite);
  }
  ui->redraw();
  ui->setInfo();
}

void MainScreen::keyPressed(unsigned int ch) {
  Site * site;
  std::string target;
  bool update = false;
  unsigned int pagerows = (unsigned int) (row - sitestartrow) * 0.6;
  if (gotomode) {
    if (ch >= 32 && ch <= 126) {
      for (unsigned int i = 0; i < mss.size(); i++) {
        Pointer<MenuSelectSiteElement> msse = mss.getSiteElement(i);
        if (toupper(ch) == toupper(msse->getSite()->getName()[0])) {
          mss.setPointer(i);
          if (mss.getSelectionPointer() >= currentviewspan + row - sitestartrow ||
              mss.getSelectionPointer() < currentviewspan) {
            ui->redraw();
          }
          break;
        }
      }
    }
    gotomode = false;
    ui->update();
    ui->setLegend();
    return;
  }
  switch(ch) {
    case KEY_UP:
      if (focusedarea->goUp()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          if (msot.size() && defocusedarea == &mss) {
            focusedarea = &msot;
          }
          else if (mso.size()) {
            focusedarea = &mso;
          }
          focusedarea->enterFocusFrom(2);
          ui->update();
          ui->setLegend();
        }
        else {
          if (mss.getSelectionPointer() < currentviewspan) {
            ui->redraw();
          }
          else {
            ui->update();
          }
        }
      }
      break;
    case KEY_DOWN:
      if (focusedarea->goDown()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          if (msot.size() && defocusedarea == &mso) {
              focusedarea = &msot;
          }
          else if (mss.size()){
            focusedarea = &mss;
          }
          focusedarea->enterFocusFrom(0);
          ui->update();
          ui->setLegend();
        }
        else {
          if (mss.getSelectionPointer() >= currentviewspan + row - sitestartrow) {
            ui->redraw();
          }
          else {
            ui->update();
          }
        }
      }
      break;
    case ' ':
    case 10:
      if (mss.isFocused()) {
        if (mss.getSite() == NULL) break;
        ui->goSiteStatus(mss.getSite()->getName());
      }
      else if (mso.isFocused() && mso.size() > 0) {
        target = mso.getElement(mso.getSelectionPointer())->getContentText();
        ui->goRaceStatus(target);
      }
      else if (msot.isFocused()) {
        target = msot.getElement(msot.getSelectionPointer())->getContentText();
        ui->goTransferJobStatus(target);
      }
      break;
    case 'G':
      ui->goGlobalOptions();
      break;
    case 'l':
      ui->goEventLog();
      break;
    case 'o':
      ui->goScoreBoard();
      break;
    case 't':
      ui->goTransfers();
      break;
    case 'r':
      ui->goAllRaces();
      break;
    case 'j':
      ui->goAllTransferJobs();
      break;
    case 'U':
    {
      bool enabled = global->getRemoteCommandHandler()->isEnabled();
      global->getRemoteCommandHandler()->setEnabled(!enabled);
      break;
    }
    case 'c':
      ui->goBrowseLocal();
      break;
    case 27: // esc
      ui->goContinueBrowsing();
      break;
  }
  if (mss.isFocused()) {
    switch(ch) {
      case 'E':
        if (mss.getSite() == NULL) break;
        ui->goEditSite(mss.getSite()->getName());
        break;
      case 'A':
        ui->goAddSite();
        break;
      case 'C':
        if (mss.getSite() == NULL) break;
        site = new Site(*mss.getSite());
        int i;
        for (i = 0; global->getSiteManager()->getSite(site->getName() + "-" + util::int2Str(i)) != NULL; i++);
        site->setName(site->getName() + "-" + util::int2Str(i));
        global->getSiteManager()->addSite(site);
        ui->redraw();
        ui->setInfo();
        break;
      case KEY_DC:
      case 'D':
        site = mss.getSite();
        if (site == NULL) break;
        deletesite = site->getName();
        ui->goConfirmation("Do you really want to delete site " + deletesite);
        break;
      case 'b':
      case KEY_RIGHT:
        if (mss.getSite() == NULL) break;
        ui->goBrowse(mss.getSite()->getName());
        break;
      case '\t':
        if (mss.getSite() == NULL) break;
        ui->goBrowseSplit(mss.getSite()->getName());
        break;
      case 'w':
        if (mss.getSite() == NULL) break;
        ui->goRawCommand(mss.getSite()->getName());
        break;
      case 'q':
        gotomode = true;
        ui->update();
        ui->setLegend();
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
          ui->redraw();
        }
        else if (update) {
          ui->update();
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
          ui->redraw();
        }
        else if (update) {
          ui->update();
        }
        break;
    }
  }
}

std::string MainScreen::getLegendText() const {
  if (gotomode) {
    return gotolegendtext;
  }
  if (focusedarea == &mss) {
    return baselegendtext + siteextralegendtext;
  }
  return baselegendtext;
}

std::string MainScreen::getInfoLabel() const {
  return "CBFTP MAIN";
}

std::string MainScreen::getInfoText() const {
  return activeracestext + activejobstext + numsitestext;
}
