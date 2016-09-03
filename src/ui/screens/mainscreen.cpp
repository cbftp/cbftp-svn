#include "mainscreen.h"

#include <cctype>

#include "../../core/pointer.h"
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
#include "../../settingsloadersaver.h"
#include "../../preparedrace.h"

#include "../menuselectoptioncheckbox.h"
#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectsiteelement.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../misc.h"

#include "allracesscreen.h"
#include "alltransferjobsscreen.h"

MainScreen::MainScreen(Ui * ui) {
  this->ui = ui;
}

void MainScreen::initialize(unsigned int row, unsigned int col) {
  std::string baselegendtext = "[Down] Next option - [Up] Previous option - [G]lobal settings - Event [l]og - [t]ransfers - All [r]aces - All transfer[j]obs - toggle [U]dp - Browse lo[c]al - [Esc] back to browsing";
  sitelegendtext = baselegendtext + " - [Tab] split browse - [right/b]rowse site - ra[w] command - [A]dd site - [E]dit site - [C]opy site - [D]elete site - [q]uick jump";
  preparelegendtext = baselegendtext + " - [Enter/s] start race - [Del] delete race";
  joblegendtext = baselegendtext + " - [Enter] Details";
  gotolegendtext = "[Any] Go to matching first letter in site list - [Esc] Cancel";
  autoupdate = true;
  gotomode = false;
  currentviewspan = 0;
  sitestartrow = 0;
  currentraces = 0;
  currenttransferjobs = 0;
  if (global->getEngine()->preparedRaces()) {
    focusedarea = &msop;
    msop.enterFocusFrom(0);
  }
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
  int listpreparedraces = global->getEngine()->preparedRaces();
  int listraces = global->getEngine()->allRaces();
  int listtransferjobs = global->getEngine()->allTransferJobs();
  unsigned int irow = 0;
  msop.clear();
  mso.clear();
  msot.clear();
  if (listpreparedraces) {
    addPreparedRaceTableHeader(irow++, msop);
    std::list<Pointer<PreparedRace> >::const_iterator it;
    int i = 0;
    for (it = --global->getEngine()->getPreparedRacesEnd(); it != --global->getEngine()->getPreparedRacesBegin() && i < 3; it--, i++) {
      addPreparedRaceDetails(irow++, msop, *it);
    }
    msop.checkPointer();
    msop.adjustLines(col - 3);
    irow++;
  }
  if (listraces) {

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
  msop.makeLeavableDown(listraces || listtransferjobs || mss.size());
  mso.makeLeavableUp(listpreparedraces);
  mso.makeLeavableDown(listtransferjobs || mss.size());
  msot.makeLeavableUp(listpreparedraces || listraces);
  msot.makeLeavableDown(mss.size());
  mss.makeLeavableUp(listpreparedraces || listraces || listtransferjobs);

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
  adaptViewSpan(currentviewspan, row - sitestartrow, position, mss.size());

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
  if (focusedarea == &msop && !msop.size()) {
    msop.reset();
    focusedarea = &mso;
    focusedarea->enterFocusFrom(0);
  }
  if (focusedarea == &mso && !mso.size()) {
    mso.reset();
    focusedarea = &msot;
    focusedarea->enterFocusFrom(0);
  }
  if (focusedarea == &msot && !msot.size()) {
    msot.reset();
    focusedarea = &mss;
    focusedarea->enterFocusFrom(0);
  }
  bool highlight;
  for (unsigned int i = 0; i < msop.size(); i++) {
    Pointer<ResizableElement> re = msop.getElement(i);
    highlight = false;
    if (msop.isFocused() && msop.getSelectionPointer() == i && listpreparedraces) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
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

  printSlider(ui, row, sitestartrow, col - 1, mss.size(), currentviewspan);
}

void MainScreen::update() {
  redraw();
}

void MainScreen::command(std::string command) {
  if (command == "yes") {
    global->getSiteLogicManager()->deleteSiteLogic(deletesite);
    global->getSiteManager()->deleteSite(deletesite);
    global->getSettingsLoaderSaver()->saveSettings();
  }
  ui->redraw();
  ui->setInfo();
}

bool MainScreen::keyPressed(unsigned int ch) {
  Site * site;
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
    return true;
  }
  switch(ch) {
    case KEY_UP:
      if (focusedarea->goUp()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          if ((defocusedarea == &mss && !msot.size() && !mso.size()) ||
              (defocusedarea == &msot && !mso.size()) ||
              defocusedarea == &mso)
          {
            focusedarea = &msop;
          }
          else if ((defocusedarea == &mss && !msot.size()) ||
                   defocusedarea == &msot)
          {
            focusedarea = &mso;
          }
          else {
            focusedarea = &msot;
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
      return true;
    case KEY_DOWN:
      if (focusedarea->goDown()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          if ((defocusedarea == &msop && !mso.size() && !msot.size()) ||
              (defocusedarea == &mso && !msot.size()) ||
              defocusedarea == &msot)
          {
            focusedarea = &mss;
          }
          else if ((defocusedarea == &msop && !mso.size()) ||
                   defocusedarea == &mso)
          {
            focusedarea = &msot;
          }
          else {
            focusedarea = &mso;
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
      return true;
    case ' ':
    case 10:
      if (mss.isFocused()) {
        if (mss.getSite() == NULL) break;
        ui->goSiteStatus(mss.getSite()->getName());
      }
      else if (mso.isFocused() && mso.size() > 0) {
        unsigned int id = mso.getElement(mso.getSelectionPointer())->getId();
        ui->goRaceStatus(id);
      }
      else if (msot.isFocused()) {
        unsigned int id = msot.getElement(msot.getSelectionPointer())->getId();
        ui->goTransferJobStatus(id);
      }
      else if (msop.isFocused()) {
        unsigned int id = msop.getElement(msop.getSelectionPointer())->getId();
        global->getEngine()->startPreparedRace(id);
        ui->redraw();
      }
      return true;
    case 'G':
      ui->goGlobalOptions();
      return true;
    case 'l':
      ui->goEventLog();
      return true;
    case 'o':
      ui->goScoreBoard();
      return true;
    case 't':
      ui->goTransfers();
      return true;
    case 'r':
      ui->goAllRaces();
      return true;
    case 's':
      if ((msop.isFocused())) {
        unsigned int id = msop.getElement(msop.getSelectionPointer())->getId();
        global->getEngine()->startPreparedRace(id);
        ui->redraw();
      }
      return true;
    case 'j':
      ui->goAllTransferJobs();
      return true;
    case 'U':
    {
      bool enabled = global->getRemoteCommandHandler()->isEnabled();
      global->getRemoteCommandHandler()->setEnabled(!enabled);
      return true;
    }
    case 'c':
      ui->goBrowseLocal();
      return true;
    case KEY_DC:
    case 'D':
      if (mss.isFocused()) {
        site = mss.getSite();
        if (site == NULL) break;
        deletesite = site->getName();
        ui->goConfirmation("Do you really want to delete site " + deletesite);
      }
      else if (msop.isFocused()) {
        unsigned int id = msop.getElement(msop.getSelectionPointer())->getId();
        global->getEngine()->deletePreparedRace(id);
        ui->redraw();
      }
      return true;
    case 27: // esc
      ui->goContinueBrowsing();
      return true;
  }
  if (mss.isFocused()) {
    switch(ch) {
      case 'E':
        if (mss.getSite() == NULL) break;
        ui->goEditSite(mss.getSite()->getName());
        return true;
      case 'A':
        ui->goAddSite();
        return true;
      case 'C':
        if (mss.getSite() == NULL) break;
        site = new Site(*mss.getSite());
        int i;
        for (i = 0; global->getSiteManager()->getSite(site->getName() + "-" + util::int2Str(i)) != NULL; i++);
        site->setName(site->getName() + "-" + util::int2Str(i));
        global->getSiteManager()->addSite(site);
        global->getSettingsLoaderSaver()->saveSettings();
        ui->redraw();
        ui->setInfo();
        return true;
      case 'b':
      case KEY_RIGHT:
        if (mss.getSite() == NULL) break;
        ui->goBrowse(mss.getSite()->getName());
        return true;
      case '\t':
        if (mss.getSite() == NULL) break;
        ui->goBrowseSplit(mss.getSite()->getName());
        return true;
      case 'w':
        if (mss.getSite() == NULL) break;
        ui->goRawCommand(mss.getSite()->getName());
        return true;
      case 'q':
        gotomode = true;
        ui->update();
        ui->setLegend();
        return true;
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
        return true;
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
        return true;
    }
  }
  return false;
}

std::string MainScreen::getLegendText() const {
  if (gotomode) {
    return gotolegendtext;
  }
  if (focusedarea == &mss) {
    return sitelegendtext;
  }
  if (focusedarea == &msop) {
    return preparelegendtext;
  }
  return joblegendtext;
}

std::string MainScreen::getInfoLabel() const {
  return "CBFTP MAIN";
}

std::string MainScreen::getInfoText() const {
  return activeracestext + activejobstext + numsitestext;
}

void MainScreen::addPreparedRaceTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id,
    bool selectable, std::string section, std::string release, std::string ttl, std::string sites)
{
  Pointer<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "section", section);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButton(y, 1, "release", release);
  if (!selectable) {
    msotb->setSelectable(false);
  }
  msotb->setId(id);
  msal->addElement(msotb, 4, 1, RESIZE_CUTEND, true);

  msotb = mso.addTextButtonNoContent(y, 1, "ttl", ttl);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "sites", sites);
  msotb->setSelectable(false);
  msal->addElement(msotb, 0, RESIZE_WITHDOTS);
}

void MainScreen::addPreparedRaceTableHeader(unsigned int y, MenuSelectOption & mso) {
  addPreparedRaceTableRow(y, mso, -1, false, "SECTION", "PREPARED RACE NAME", "EXPIRES", "SITES");
}

void MainScreen::addPreparedRaceDetails(unsigned int y, MenuSelectOption & mso, const Pointer<PreparedRace> & preparedrace) {
  unsigned int id = preparedrace->getId();
  std::string section = preparedrace->getSection();
  std::string release = preparedrace->getRelease();
  std::string ttl = util::simpleTimeFormat(preparedrace->getRemainingTime());
  std::list<std::string> sites = preparedrace->getSites();
  std::string sitestr;
  for (std::list<std::string>::const_iterator it = sites.begin(); it != sites.end(); it++) {
    sitestr += *it + ",";
  }
  if (sitestr.length() > 0) {
    sitestr = sitestr.substr(0, sitestr.length() - 1);
  }
  addPreparedRaceTableRow(y, mso, id, true, section, release, ttl, sitestr);
}
