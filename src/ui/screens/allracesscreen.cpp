#include "allracesscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../misc.h"

#include "../../core/tickpoke.h"

#include "../../race.h"
#include "../../engine.h"
#include "../../globalcontext.h"
#include "../../util.h"

namespace {

enum KeyAction {
  KEYACTION_RESET_HARD,
  KEYACTION_ABORT_DELETE_INC,
  KEYACTION_ABORT_DELETE_ALL
};

bool filteringActive(const SpreadJobsFilteringParameters& sjfp) {
  bool jobnamefilter = sjfp.usejobnamefilter && !sjfp.jobnamefilter.empty();
  bool sitefilter = sjfp.usesitefilter && (!sjfp.anysitefilters.empty() || !sjfp.allsitefilters.empty());
  bool statusfilter = sjfp.usestatusfilter && (sjfp.showstatusinprogress || sjfp.showstatusdone || sjfp.showstatustimeout || sjfp.showstatusaborted);
  return jobnamefilter || sitefilter || statusfilter;
}

std::string getFilterText(const SpreadJobsFilteringParameters& sjfp) {
  std::string output;
  int filters = 0;
  bool jobnamefilter = sjfp.usejobnamefilter && !sjfp.jobnamefilter.empty();
  bool sitefilter = sjfp.usesitefilter && (!sjfp.anysitefilters.empty() || !sjfp.allsitefilters.empty());
  bool statusfilter = sjfp.usestatusfilter && (sjfp.showstatusinprogress || sjfp.showstatusdone || sjfp.showstatustimeout || sjfp.showstatusaborted);
  if (jobnamefilter) {
    filters++;
  }
  if (sitefilter) {
    filters++;
  }
  if (statusfilter) {
    filters++;
  }
  if (jobnamefilter) {
    if (filters > 1) {
      output += "jobname,";
    }
    else {
      output += "jobname: " + sjfp.jobnamefilter;
    }
  }
  if (sitefilter) {
    int sitefilters = 0;
    if (!sjfp.anysitefilters.empty()) {
      sitefilters++;
    }
    if (!sjfp.allsitefilters.empty()) {
      sitefilters++;
    }
    if (filters > 1 || sitefilters > 1) {
      output += "sites";
      if (filters > 1) {
        output += ",";
      }
    }
    else {
      if (!sjfp.anysitefilters.empty()) {
        std::string sitelist = util::join(sjfp.anysitefilters, ",");
        if (sitelist.length() > 10) {
          output += "any sites";
        }
        else {
          output += "site";
          output += (sjfp.anysitefilters.size() > 1 ? "s: " : ": ") + sitelist;
        }
      }
      else if (!sjfp.allsitefilters.empty()) {
        std::string sitelist = util::join(sjfp.allsitefilters, ",");
        if (sitelist.length() > 10) {
          output += "all sites";
        }
        else {
          output += "site";
          output += (sjfp.allsitefilters.size() > 1 ? "s: " : ": ") + sitelist;
        }
      }
    }
  }
  if (statusfilter) {
    if (filters > 1) {
      output += "status,";
    }
    else {
      output += "status: ";
      std::string statustext;
      if (sjfp.showstatusinprogress) {
        statustext += "inprogress,";
      }
      if (sjfp.showstatusdone) {
        statustext += "done,";
      }
      if (sjfp.showstatustimeout) {
        statustext += "timeout,";
      }
      if (sjfp.showstatusaborted) {
        statustext += "aborted,";
      }
      if (statustext.length()) {
        output += statustext.substr(0, statustext.length() - 1);
      }
    }
  }
  if (filters > 1) {
    output = output.substr(0, output.length() - 1);
  }
  return output;
}

}

AllRacesScreen::AllRacesScreen(Ui* ui) : UIWindow(ui, "AllRacesScreen"), table(*vv) {
  keybinds.addBind(10, KEYACTION_ENTER, "Details");
  keybinds.addBind('r', KEYACTION_RESET, "Reset job");
  keybinds.addBind('R', KEYACTION_RESET_HARD, "Hard reset job");
  keybinds.addBind('B', KEYACTION_ABORT, "Abort job");
  keybinds.addBind('f', KEYACTION_FILTER, "Toggle filtering");
  keybinds.addBind('q', KEYACTION_QUICK_JUMP, "Quick jump");
  keybinds.addBind('t', KEYACTION_TRANSFERS, "Transfers for job");
  keybinds.addBind('z', KEYACTION_ABORT_DELETE_INC, "Abort and delete own files on incomplete sites");
  keybinds.addBind('Z', KEYACTION_ABORT_DELETE_ALL, "Abort and delete own files on ALL sites");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Return");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Navigate up");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Navigate down");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Next page");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Previous page");
  keybinds.addBind(KEY_HOME, KEYACTION_TOP, "Go top");
  keybinds.addBind(KEY_END, KEYACTION_BOTTOM, "Go bottom");
  keybinds.addBind('-', KEYACTION_HIGHLIGHT_LINE, "Highlight entire line");
}

AllRacesScreen::~AllRacesScreen() {
  disableGotoMode();
}

void AllRacesScreen::initialize(unsigned int row, unsigned int col) {
  initialize(row, col, SpreadJobsFilteringParameters());
}

void AllRacesScreen::initializeFilterSite(unsigned int row, unsigned int col, const std::string& site) {
  SpreadJobsFilteringParameters sjfp;
  sjfp.usesitefilter = true;
  sjfp.anysitefilters.push_back(site);
  initialize(row, col, sjfp);
}

void AllRacesScreen::initialize(unsigned int row, unsigned int col, const SpreadJobsFilteringParameters& sjfp) {
  filtering = filteringActive(sjfp);
  this->sjfp = sjfp;
  autoupdate = true;
  hascontents = false;
  currentviewspan = 0;
  ypos = 0;
  temphighlightline = false;
  engine = global->getEngine();
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

bool AllRacesScreen::showsWhileFiltered(const std::shared_ptr<Race>& race) const {
  if (sjfp.usejobnamefilter && !sjfp.jobnamefilter.empty()) {
    if (!util::wildcmp(sjfp.jobnamefilter.c_str(), race->getName().c_str())) {
      return false;
    }
  }
  if (sjfp.usesitefilter) {
    if (!sjfp.anysitefilters.empty()) {
      bool match = false;
      for (std::list<std::string>::const_iterator it = sjfp.anysitefilters.begin(); it != sjfp.anysitefilters.end(); it++) {
        if (race->getSiteRace(*it)) {
          match = true;
          break;
        }
      }
      if (!match) {
        return false;
      }
    }
    if (!sjfp.allsitefilters.empty()) {
      bool match = true;
      for (std::list<std::string>::const_iterator it = sjfp.allsitefilters.begin(); it != sjfp.allsitefilters.end(); it++) {
        if (!race->getSiteRace(*it)) {
          match = false;
          break;
        }
      }
      if (!match) {
        return false;
      }
    }
  }

  if (sjfp.usestatusfilter && (sjfp.showstatusinprogress || sjfp.showstatusdone || sjfp.showstatustimeout || sjfp.showstatusaborted)) {
    switch (race->getStatus()) {
      case RaceStatus::RUNNING:
        if (!sjfp.showstatusinprogress) {
          return false;
        }
        break;
      case RaceStatus::DONE:
        if (!sjfp.showstatusdone) {
          return false;
        }
        break;
      case RaceStatus::ABORTED:
        if (!sjfp.showstatusaborted) {
          return false;
        }
        break;
      case RaceStatus::TIMEOUT:
        if (!sjfp.showstatustimeout) {
          return false;
        }
        break;
    }
  }
  return true;
}

unsigned int AllRacesScreen::totalListSize() const {
  int filteredcount = 0;
  if (filtering) {
    for (std::list<std::shared_ptr<Race>>::const_iterator it = engine->getRacesBegin(); it != engine->getRacesEnd(); ++it) {
      if (showsWhileFiltered(*it)) {
        filteredcount++;
      }
    }
  }
  return filtering ? filteredcount : engine->allRaces();
}

void AllRacesScreen::redraw() {
  vv->clear();
  unsigned int y = 0;
  unsigned int listspan = row - 1;
  unsigned int totallistsize = totalListSize();
  table.reset();
  while (ypos && ypos >= engine->allRaces()) {
    --ypos;
  }
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addRaceTableHeader(y, table, "RELEASE");
  y++;
  unsigned int pos = 0;
  for (std::list<std::shared_ptr<Race> >::const_iterator it = --engine->getCurrentRacesEnd(); it != --engine->getCurrentRacesBegin() && y < row; it--) {
    if (filtering && !showsWhileFiltered(*it)) {
      continue;
    }
    if (pos >= currentviewspan) {
      addRaceDetails(y++, table, *it);
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
  }
  for (std::list<std::shared_ptr<Race> >::const_iterator it = --engine->getFinishedRacesEnd(); it != --engine->getFinishedRacesBegin() && y < row; it--) {
    if (filtering && !showsWhileFiltered(*it)) {
      continue;
    }
    if (pos >= currentviewspan) {
      addRaceDetails(y++, table, *it);
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
  }
  table.checkPointer();
  hascontents = table.linesSize() > 1;
  table.adjustLines(col - 3);
  std::shared_ptr<MenuSelectAdjustableLine> highlightline;
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    bool highlight = hascontents && table.getSelectionPointer() == i;
    if (re->isVisible()) {
      vv->putStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
    if (highlight && (temphighlightline ^ ui->getHighlightEntireLine())) {
      highlightline = table.getAdjustableLine(re);
    }
  }
  if (highlightline) {
    std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol();
    vv->highlightOn(highlightline->getRow(), minmaxcol.first, minmaxcol.second - minmaxcol.first + 1);
  }
  printSlider(vv, row, 1, col - 1, totallistsize, currentviewspan);
}

void AllRacesScreen::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    if (!!abortrace) {
      global->getEngine()->abortRace(abortrace);
    }
    else if (!!abortdeleteraceinc) {
      global->getEngine()->deleteOnAllSites(abortdeleteraceinc, false, false);
    }
    else if (!!abortdeleteraceall) {
      global->getEngine()->deleteOnAllSites(abortdeleteraceall, false, true);
    }
    ui->update();
  }
  abortrace.reset();
  abortdeleteraceinc.reset();
  abortdeleteraceall.reset();
}

bool AllRacesScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (temphighlightline) {
    temphighlightline = false;
    ui->redraw();
    if (action == KEYACTION_HIGHLIGHT_LINE) {
      return true;
    }
  }
  if (gotomode) {
      if (gotomodefirst) {
        gotomodefirst = false;
      }
      if (ch >= 32 && ch <= 126) {
        gotomodeticker = 0;
        gotomodestring += toupper(ch);
        unsigned int gotomodelength = gotomodestring.length();
        unsigned int pos = 0;
        bool found = false;
        for (std::list<std::shared_ptr<Race> >::const_iterator it = --engine->getCurrentRacesEnd(); it != --engine->getCurrentRacesBegin(); it--) {
          if (filtering && !showsWhileFiltered(*it)) {
            continue;
          }
          std::string name = (*it)->getName();
          if (name.length() >= gotomodelength) {
            std::string substr = name.substr(0, gotomodelength);
              for (unsigned int j = 0; j < gotomodelength; j++) {
                substr[j] = toupper(substr[j]);
              }
              if (substr == gotomodestring) {
                ypos = pos;
                found = true;
                break;
              }
          }
          ++pos;
        }
        if (!found) {
          for (std::list<std::shared_ptr<Race> >::const_iterator it = --engine->getFinishedRacesEnd(); it != --engine->getFinishedRacesBegin(); it--) {
            if (filtering && !showsWhileFiltered(*it)) {
              continue;
            }
            std::string name = (*it)->getName();
            if (name.length() >= gotomodelength) {
              std::string substr = name.substr(0, gotomodelength);
                for (unsigned int j = 0; j < gotomodelength; j++) {
                  substr[j] = toupper(substr[j]);
                }
                if (substr == gotomodestring) {
                  ypos = pos;
                  break;
                }
            }
            ++pos;
          }
        }
        ui->redraw();
        return true;
      }
      else {
        disableGotoMode();
      }
      if (ch == 27) {
        return false;
      }
    }
  switch (action) {
    case KEYACTION_UP:
      if (hascontents && ypos > 0) {
        --ypos;
        table.goUp();
        ui->update();
      }
      return true;
    case KEYACTION_DOWN:
      if (hascontents && ypos < engine->allRaces() - 1) {
        ++ypos;
        table.goDown();
        ui->update();
      }
      return true;
    case KEYACTION_NEXT_PAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos < engine->allRaces() - 1; i++) {
        ypos++;
        table.goDown();
      }
      ui->update();
      return true;
    }
    case KEYACTION_PREVIOUS_PAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos > 0; i++) {
        ypos--;
        table.goUp();
      }
      ui->update();
      return true;
    }
    case KEYACTION_TOP:
      ypos = 0;
      ui->update();
      return true;
    case KEYACTION_BOTTOM:
      ypos = engine->allRaces() - 1;
      ui->update();
      return true;
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
    case KEYACTION_ENTER:
      if (hascontents) {
        ui->goRaceStatus(table.getElement(table.getSelectionPointer())->getId());
      }
      return true;
    case KEYACTION_QUICK_JUMP:
      gotomode = true;
      gotomodefirst = true;
      gotomodeticker = 0;
      gotomodestring = "";
      global->getTickPoke()->startPoke(this, "AllRacesScreen", 50, 0);
      ui->update();
      ui->setLegend();
      break;
    case KEYACTION_FILTER:
      if (!filtering) {
        ui->goSpreadJobsFiltering(sjfp);
      }
      else {
        filtering = false;
        ui->setInfo();
        ui->redraw();
      }
      return true;
    case KEYACTION_ABORT:
      if (hascontents) {
        abortrace = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!abortrace && abortrace->getStatus() == RaceStatus::RUNNING) {
          ui->goConfirmation("Do you really want to abort the spread job " + abortrace->getName());
        }
      }
      return true;
    case KEYACTION_ABORT_DELETE_INC:
      if (hascontents) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!race && race->getStatus() == RaceStatus::RUNNING) {
          abortdeleteraceinc = race;
          ui->goConfirmation("Do you really want to abort the race " + abortdeleteraceinc->getName() + " and delete your own files on all incomplete sites?");
        }
      }
      return true;
    case KEYACTION_ABORT_DELETE_ALL:
      if (hascontents) {
        abortdeleteraceall = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!abortdeleteraceall) {
          if (abortdeleteraceall->getStatus() == RaceStatus::RUNNING) {
            ui->goConfirmation("Do you really want to abort the race " + abortdeleteraceall->getName() + " and delete your own files on ALL involved sites?");
          }
          else {
            ui->goConfirmation("Do you really want to delete your own files in " + abortdeleteraceall->getName() + " on ALL involved sites?");
          }
        }
      }
      return true;
    case KEYACTION_RESET:
    case KEYACTION_RESET_HARD:
      if (hascontents) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!race) {
          global->getEngine()->resetRace(race, action == KEYACTION_RESET_HARD);
        }
      }
      return true;
    case KEYACTION_TRANSFERS:
      if (hascontents) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!race) {
          ui->goTransfersFilterSpreadJob(race->getName());
        }
      }
      return true;
    case KEYACTION_HIGHLIGHT_LINE:
      if (!hascontents) {
        break;
      }
      temphighlightline = true;
      ui->redraw();
      return true;
  }
  return false;
}

void AllRacesScreen::tick(int) {
  if (gotomode && !gotomodefirst) {
    if (gotomodeticker++ >= 20) {
      disableGotoMode();
    }
  }
}

void AllRacesScreen::disableGotoMode() {
  if (gotomode) {
    gotomode = false;
    global->getTickPoke()->stopPoke(this, 0);
    ui->setLegend();
  }
}

std::string AllRacesScreen::getInfoLabel() const {
  return "ALL SPREAD JOBS";
}

std::string AllRacesScreen::getInfoText() const {
  if (filtering) {
    return "FILTERING: " + getFilterText(sjfp);
  }
  return "Active: " + std::to_string(engine->currentRaces()) + "  Total: " + std::to_string(engine->allRaces());
}

std::string AllRacesScreen::getLegendText() const {
  if (gotomode) {
    return "[Any] Go to first matching entry name - [Esc] Cancel";
  }
  return keybinds.getLegendSummary();
}

void AllRacesScreen::addRaceTableHeader(unsigned int y, MenuSelectOption & mso, const std::string & release) {
  addRaceTableRow(y, mso, -1, false, "STARTED", "USE", "SECTION", release, "SIZE", "WORST", "AVG", "BEST", "STATUS", "DONE", "SITES");
}

void AllRacesScreen::addRaceTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id, bool selectable,
    const std::string & timestamp, const std::string & timespent, const std::string & section, const std::string & release,
    const std::string & size, const std::string & worst, const std::string & avg, const std::string & best, const std::string & status, const std::string & done,
    const std::string & sites)
{
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "timestamp", timestamp);
  msotb->setSelectable(false);
  msal->addElement(msotb, 7, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timespent", timespent);
  msotb->setSelectable(false);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "section", section);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "release", release);
  msotb->setSelectable(selectable);
  msotb->setId(id);
  msal->addElement(msotb, 12, 1, RESIZE_CUTEND, true);

  msotb = mso.addTextButtonNoContent(y, 1, "size", size);
  msotb->setSelectable(false);
  msal->addElement(msotb, 10, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "worst", worst);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "avg", avg);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "best", best);
  msotb->setSelectable(false);
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "status", status);
  msotb->setSelectable(false);
  msal->addElement(msotb, 11, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "done", done);
  msotb->setSelectable(false);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "sites", sites);
  msotb->setSelectable(false);
  msal->addElement(msotb, 0, RESIZE_WITHDOTS);
}

void AllRacesScreen::addRaceDetails(unsigned int y, MenuSelectOption & mso, std::shared_ptr<Race> race) {
  std::string done = std::to_string(race->numSitesDone()) + "/" + std::to_string(race->numSites());
  std::string timespent = util::simpleTimeFormat(race->getTimeSpent());
  std::string status;
  std::string worst = std::to_string(race->getWorstCompletionPercentage()) + "%";
  std::string avg = std::to_string(race->getAverageCompletionPercentage()) + "%";
  std::string best = std::to_string(race->getBestCompletionPercentage()) + "%";
  std::string size = util::parseSize(race->estimatedTotalSize());
  switch (race->getStatus()) {
    case RaceStatus::RUNNING:
      status = "running";
      break;
    case RaceStatus::DONE:
      status = "done";
      break;
    case RaceStatus::ABORTED:
      status = "aborted";
      break;
    case RaceStatus::TIMEOUT:
      status = "timeout";
      break;
  }
  addRaceTableRow(y, mso, race->getId(), true, race->getTimeStamp(), timespent, race->getSection(),
                  race->getName(), size, worst, avg, best, status, done,
                  race->getSiteListText());
}
