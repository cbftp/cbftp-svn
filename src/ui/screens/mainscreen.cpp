#include "mainscreen.h"

#include <cctype>

#include "../../core/pointer.h"
#include "../../globalcontext.h"
#include "../../site.h"
#include "../../race.h"
#include "../../engine.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../transferjob.h"
#include "../../util.h"
#include "../../remotecommandhandler.h"
#include "../../settingsloadersaver.h"
#include "../../preparedrace.h"

#include "../menuselectoptioncheckbox.h"
#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../misc.h"

#include "allracesscreen.h"
#include "alltransferjobsscreen.h"

MainScreen::MainScreen(Ui * ui) {
  this->ui = ui;
}

void MainScreen::initialize(unsigned int row, unsigned int col) {
  std::string baselegendtext = "[Down] Next option - [Up] Previous option - [G]lobal settings - Event [l]og - [t]ransfers - All [r]aces - All transfer[j]obs - toggle [U]dp - Browse lo[c]al - General [i]nfo - [Esc] back to browsing";
  sitelegendtext = baselegendtext + " - [Tab] split browse - [right/b]rowse site - ra[w] command - [A]dd site - [E]dit site - [C]opy site - [D]elete site - [q]uick jump -[L]ogin all slots";
  preparelegendtext = baselegendtext + " - [Enter/s] start race - [Del] delete race";
  joblegendtext = baselegendtext + " - [Enter] Details";
  gotolegendtext = "[Any] Go to matching first letter in site list - [Esc] Cancel";
  autoupdate = true;
  gotomode = false;
  currentviewspan = 0;
  sitestartrow = 0;
  currentraces = 0;
  currenttransferjobs = 0;
  sitepos = 0;
  if (global->getEngine()->preparedRaces()) {
    focusedarea = &msop;
    msop.enterFocusFrom(0);
  }
  else if (global->getEngine()->currentRaces()) {
    focusedarea = &msosj;
    msosj.enterFocusFrom(0);
  }
  else if (global->getEngine()->currentTransferJobs()) {
    focusedarea = &msotj;
    msotj.enterFocusFrom(0);
  }
  else {
    focusedarea = &msos;
    msos.enterFocusFrom(0);
  }
  init(row, col);
}

void MainScreen::redraw() {
  ui->erase();
  ui->hideCursor();
  totalsitessize = global->getSiteManager()->getNumSites();
  numsitestext = "Sites: " + util::int2Str(totalsitessize);
  int listpreparedraces = global->getEngine()->preparedRaces();
  int listraces = global->getEngine()->allRaces();
  int listtransferjobs = global->getEngine()->allTransferJobs();

  unsigned int irow = 0;
  msop.clear();
  msosj.clear();
  msotj.clear();
  msos.clear();
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

    AllRacesScreen::addRaceTableHeader(irow++, msosj, std::string("RACE NAME") + (listraces > 3 ? " (Showing latest 3)" : ""));
    std::list<Pointer<Race> >::const_iterator it;
    int i = 0;
    for (it = --global->getEngine()->getRacesEnd(); it != --global->getEngine()->getRacesBegin() && i < 3; it--, i++) {
      AllRacesScreen::addRaceDetails(irow++, msosj, *it);
    }
    msosj.checkPointer();
    msosj.adjustLines(col - 3);
    irow++;
  }
  if (listtransferjobs) {

    AllTransferJobsScreen::addJobTableHeader(irow++, msotj, std::string("TRANSFER JOB NAME") + (listtransferjobs > 3 ? " (Showing latest 3)" : ""));
    std::list<Pointer<TransferJob> >::const_iterator it;
    int i = 0;
    for (it = --global->getEngine()->getTransferJobsEnd(); it != --global->getEngine()->getTransferJobsBegin() && i < 3; it--, i++) {
      AllTransferJobsScreen::addJobDetails(irow++, msotj, *it);
    }
    msotj.checkPointer();
    msotj.adjustLines(col - 3);
    irow++;
  }
  msop.makeLeavableDown(listraces || listtransferjobs || totalsitessize);
  msosj.makeLeavableUp(listpreparedraces);
  msosj.makeLeavableDown(listtransferjobs || totalsitessize);
  msotj.makeLeavableUp(listpreparedraces || listraces);
  msotj.makeLeavableDown(totalsitessize);
  msos.makeLeavableUp(listpreparedraces || listraces || listtransferjobs);

  if (!totalsitessize) {
    ui->printStr(irow, 1, "Press 'A' to add a site");
  }
  int y = irow;
  if (totalsitessize) {
    addSiteHeader(y++, msos);
    sitestartrow = y;
    adaptViewSpan(currentviewspan, row - sitestartrow, sitepos, totalsitessize);
    for (unsigned int i = currentviewspan; (int)i < global->getSiteManager()->getNumSites() && i - currentviewspan < row - sitestartrow; ++i) {
      Pointer<Site> site = global->getSiteManager()->getSite(i);
      Pointer<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(site->getName());
      addSiteDetails(y++, msos, sl);
    }

    msos.setPointer(msos.getAdjustableLine(sitepos + 1 - currentviewspan)->getElement(0));
    msos.checkPointer();
    msos.adjustLines(col - 3);

    printSlider(ui, row, sitestartrow, col - 1, totalsitessize, currentviewspan);
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
  if (focusedarea == &msop && !msop.size()) {
    msop.reset();
    focusedarea = &msosj;
    focusedarea->enterFocusFrom(0);
  }
  if (focusedarea == &msosj && !msosj.size()) {
    msosj.reset();
    focusedarea = &msotj;
    focusedarea->enterFocusFrom(0);
  }
  if (focusedarea == &msotj && !msotj.size()) {
    msotj.reset();
    focusedarea = &msos;
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
  for (unsigned int i = 0; i < msosj.size(); i++) {
    Pointer<ResizableElement> re = msosj.getElement(i);
    highlight = false;
    if (msosj.isFocused() && msosj.getSelectionPointer() == i && listraces) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
  for (unsigned int i = 0; i < msotj.size(); i++) {
    Pointer<ResizableElement> re = msotj.getElement(i);
    highlight = false;
    if (msotj.isFocused() && msotj.getSelectionPointer() == i && listtransferjobs) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
  for (unsigned int i = 0; i < msos.size(); i++) {
    Pointer<ResizableElement> re = msos.getElement(i);
    highlight = false;
    if (msos.isFocused() && msos.getSelectionPointer() == i && totalsitessize) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
}

void MainScreen::update() {
  redraw();
  ui->setInfo();
}

void MainScreen::command(const std::string & command) {
  if (command == "yes") {
    global->getSiteLogicManager()->deleteSiteLogic(deletesite);
    global->getSiteManager()->deleteSite(deletesite);
    global->getSettingsLoaderSaver()->saveSettings();
  }
  ui->redraw();
  ui->setInfo();
}

void MainScreen::keyUp() {
  if (focusedarea == &msos) {
    if (sitepos) {
      --sitepos;
    }
    else if (msotj.size() || msosj.size() || msop.size()) {
      msos.defocus();
    }
  }
  else {
    focusedarea->goUp();
  }
  if (!focusedarea->isFocused()) {
    defocusedarea = focusedarea;
    if ((defocusedarea == &msos && !msotj.size() && !msosj.size()) ||
        (defocusedarea == &msotj && !msosj.size()) ||
        defocusedarea == &msosj)
    {
      focusedarea = &msop;
    }
    else if ((defocusedarea == &msos && !msotj.size()) ||
             defocusedarea == &msotj)
    {
      focusedarea = &msosj;
    }
    else {
      focusedarea = &msotj;
    }
    focusedarea->enterFocusFrom(2);
    ui->setLegend();
  }
}

void MainScreen::keyDown() {
  if (focusedarea == &msos) {
    if (sitepos + 1 < totalsitessize) {
      ++sitepos;
    }
  }
  else {
    focusedarea->goDown();
  }
  if (!focusedarea->isFocused()) {
    defocusedarea = focusedarea;
    if ((defocusedarea == &msop && !msosj.size() && !msotj.size()) ||
        (defocusedarea == &msosj && !msotj.size()) ||
        defocusedarea == &msotj)
    {
      focusedarea = &msos;
    }
    else if ((defocusedarea == &msop && !msosj.size()) ||
             defocusedarea == &msosj)
    {
      focusedarea = &msotj;
    }
    else {
      focusedarea = &msos;
    }
    focusedarea->enterFocusFrom(0);
    ui->setLegend();
  }
}

bool MainScreen::keyPressed(unsigned int ch) {
  unsigned int pagerows = (unsigned int) (row - sitestartrow) * 0.6;
  if (gotomode) {
    if (ch >= 32 && ch <= 126) {
      for (int i = 0; i < global->getSiteManager()->getNumSites(); i++) {
        Pointer<Site> site = global->getSiteManager()->getSite(i);
        if (toupper(ch) == toupper(site->getName()[0])) {
          sitepos = i;
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
      keyUp();
      ui->redraw();
      return true;
    case KEY_DOWN:
      keyDown();
      ui->redraw();
      return true;
    case ' ':
    case 10:
      if (msos.isFocused()) {
        if (msos.linesSize() == 1) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getLabelText();
        Pointer<Site> site = global->getSiteManager()->getSite(sitename);
        ui->goSiteStatus(site->getName());
      }
      else if (msosj.isFocused() && msosj.size() > 0) {
        unsigned int id = msosj.getElement(msosj.getSelectionPointer())->getId();
        ui->goRaceStatus(id);
      }
      else if (msotj.isFocused()) {
        unsigned int id = msotj.getElement(msotj.getSelectionPointer())->getId();
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
    case 'i':
      ui->goInfo();
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
      if (msos.isFocused()) {
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getLabelText();
        Pointer<Site> site = global->getSiteManager()->getSite(sitename);
        if (!site) break;
        deletesite = site->getName();
        ui->goConfirmation("Do you really want to delete site " + deletesite);
      }
      else if (msop.isFocused()) {
        unsigned int id = msop.getElement(msop.getSelectionPointer())->getId();
        global->getEngine()->deletePreparedRace(id);
        ui->redraw();
      }
      return true;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        keyDown();
      }
      ui->redraw();
      return true;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        keyUp();
      }
      ui->redraw();
      return true;
    case 27: // esc
      ui->goContinueBrowsing();
      return true;
  }
  if (msos.isFocused()) {
    switch(ch) {
      case 'E': {
        if (msos.linesSize() == 1) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getLabelText();
        ui->goEditSite(sitename);
        return true;
      }
      case 'A':
        ui->goAddSite();
        return true;
      case 'C': {
        if (msos.linesSize() == 1) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getLabelText();
        Pointer<Site> oldsite = global->getSiteManager()->getSite(sitename);
        Pointer<Site> site = makePointer<Site>(*oldsite);
        int i;
        for (i = 0; !!global->getSiteManager()->getSite(site->getName() + "-" + util::int2Str(i)); i++);
        site->setName(site->getName() + "-" + util::int2Str(i));
        global->getSiteManager()->addSite(site);
        global->getSettingsLoaderSaver()->saveSettings();
        ui->redraw();
        ui->setInfo();
        return true;
      }
      case 'b':
      case KEY_RIGHT: {
        if (msos.linesSize() == 1) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getLabelText();
        ui->goBrowse(sitename);
        return true;
      }
      case 'L': {
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getLabelText();
        global->getSiteLogicManager()->getSiteLogic(sitename)->activateAll();
        return true;
      }
      case '\t': {
        if (msos.linesSize() == 1) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getLabelText();
        ui->goBrowseSplit(sitename);
        return true;
      }
      case 'w': {
        if (msos.linesSize() == 1) break;
        std::string sitename = msos.getElement(msos.getSelectionPointer())->getLabelText();
        ui->goRawCommand(sitename);
        return true;
      }
      case 'q':
        gotomode = true;
        ui->update();
        ui->setLegend();
        return true;
    }
  }
  return false;
}

std::string MainScreen::getLegendText() const {
  if (gotomode) {
    return gotolegendtext;
  }
  if (focusedarea == &msos) {
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
  std::string text;
  if (global->getRemoteCommandHandler()->isEnabled()) {
    text += "Remote commands enabled  ";
  }
  if (global->getEngine()->getNextPreparedRaceStarterEnabled()) {
    text += "Race starter: " + util::simpleTimeFormat(global->getEngine()->getNextPreparedRaceStarterTimeRemaining()) + "  ";
  }
  return text + activeracestext + activejobstext + numsitestext;
}

void MainScreen::addPreparedRaceTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id,
    bool selectable, const std::string & section, const std::string & release, const std::string & ttl,
    const std::string & sites)
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

void MainScreen::addSiteHeader(unsigned int y, MenuSelectOption & mso) {
  addSiteRow(y, mso, false, "SITE           ", "LOGINS", "UPLOADS", "DOWNLOADS", "UP", "DOWN", "DISABLED", "UP 24HR", "DOWN 24HR", "ALLUP", "ALLDOWN");
}

void MainScreen::addSiteRow(unsigned int y, MenuSelectOption & mso, bool selectable, const std::string & site,
    const std::string & logins, const std::string & uploads, const std::string & downloads,
    const std::string & allowup, const std::string & allowdown, const std::string & disabled,
    const std::string & dayup, const std::string & daydn, const std::string & alup, const std::string & aldn)
{
  Pointer<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "site", site);
  if (!selectable) {
    msotb->setSelectable(false);
  }
  msal->addElement(msotb, 12, 5, RESIZE_CUTEND, false);

  msotb = mso.addTextButtonNoContent(y, 1, "logins", logins);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 11, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "uploads", uploads);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 9, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "downloads", downloads);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 10, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "up", allowup);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 7, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "down", allowdown);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "disabled", disabled);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "dayup", dayup);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 1, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "daydn", daydn);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "alup", alup);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "aldn", aldn);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 4, RESIZE_REMOVE);
}

void MainScreen::addSiteDetails(unsigned int y, MenuSelectOption & mso, const Pointer<SiteLogic> & sl) {
  Pointer<Site> site = sl->getSite();
  std::string sitename = site->getName();
  std::string logins = util::int2Str(sl->getCurrLogins());
  if (!site->unlimitedLogins()) {
    logins += "/" + util::int2Str(site->getMaxLogins());
  }
  std::string uploads = util::int2Str(sl->getCurrUp());
  if (!site->unlimitedUp()) {
    uploads += "/" + util::int2Str(site->getMaxUp());
  }
  std::string downloads = util::int2Str(sl->getCurrDown());
  if (!site->unlimitedDown()) {
    downloads += "/" + util::int2Str(site->getMaxDown());
  }
  std::string up = site->getAllowUpload()? "[X]" : "[ ]";
  std::string down = site->getAllowDownload()? "[X]" : "[ ]";
  std::string disabled = site->getDisabled()? "[X]" : "[ ]";
  std::string up24 = util::parseSize(site->getSizeUpLast24Hours());
  std::string down24 = util::parseSize(site->getSizeDownLast24Hours());
  std::string allup = util::parseSize(site->getSizeUpAll());
  std::string alldown = util::parseSize(site->getSizeDownAll());
  addSiteRow(y, mso, true, sitename, logins, uploads, downloads, up, down, disabled, up24, down24, allup, alldown);
}
