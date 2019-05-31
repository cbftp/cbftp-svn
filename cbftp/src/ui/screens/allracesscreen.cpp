#include "allracesscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../misc.h"

#include "../../race.h"
#include "../../engine.h"
#include "../../globalcontext.h"
#include "../../util.h"

AllRacesScreen::AllRacesScreen(Ui * ui) {
  this->ui = ui;
}

void AllRacesScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  hascontents = false;
  currentviewspan = 0;
  ypos = 0;
  temphighlightline = -1;
  engine = global->getEngine();
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void AllRacesScreen::redraw() {
  ui->erase();
  unsigned int y = 0;
  unsigned int listspan = row - 1;
  unsigned int totallistsize = engine->allRaces();
  table.reset();
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addRaceTableHeader(y, table, "RELEASE");
  y++;
  unsigned int pos = 0;
  for (std::list<std::shared_ptr<Race> >::const_iterator it = --engine->getCurrentRacesEnd(); it != --engine->getCurrentRacesBegin() && y < row; it--) {
    if (pos >= currentviewspan) {
      addRaceDetails(y++, table, *it);
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
  }
  for (std::list<std::shared_ptr<Race> >::const_iterator it = --engine->getFinishedRacesEnd(); it != --engine->getFinishedRacesBegin() && y < row; it--) {
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
  if (temphighlightline != -1) {
    std::shared_ptr<MenuSelectAdjustableLine> highlightline = table.getAdjustableLineOnRow(temphighlightline);
    if (!!highlightline) {
      std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol();
      for (unsigned int i = minmaxcol.first; i <= minmaxcol.second; i++) {
        ui->printChar(temphighlightline, i, ' ', true);
      }
    }
  }
  bool highlight;
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    highlight = false;
    if (hascontents && (table.getSelectionPointer() == i  || (int)re->getRow() == temphighlightline)) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
  printSlider(ui, row, 1, col - 1, totallistsize, currentviewspan);
}

void AllRacesScreen::update() {
  redraw();
}

void AllRacesScreen::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    if (!!abortrace) {
      global->getEngine()->abortRace(abortrace);
    }
    else if (!!abortdeleterace) {
      global->getEngine()->deleteOnAllSites(abortdeleterace, false);
    }
    ui->update();
  }
  abortrace = false;
  abortdeleterace = false;
}

bool AllRacesScreen::keyPressed(unsigned int ch) {
  if (temphighlightline != -1) {
    temphighlightline = -1;
    ui->redraw();
    if (ch == '-') {
      return true;
    }
  }
  switch (ch) {
    case KEY_UP:
      if (hascontents && ypos > 0) {
        --ypos;
        table.goUp();
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (hascontents && ypos < engine->allRaces() - 1) {
        ++ypos;
        table.goDown();
        ui->update();
      }
      return true;
    case KEY_NPAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos < engine->allRaces() - 1; i++) {
        ypos++;
        table.goDown();
      }
      ui->update();
      return true;
    }
    case KEY_PPAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos > 0; i++) {
        ypos--;
        table.goUp();
      }
      ui->update();
      return true;
    }
    case KEY_HOME:
      ypos = 0;
      ui->update();
      return true;
    case KEY_END:
      ypos = engine->allRaces() - 1;
      ui->update();
      return true;
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
    case 10:
      if (hascontents) {
        ui->goRaceStatus(table.getElement(table.getSelectionPointer())->getId());
      }
      return true;
    case 'B':
      if (hascontents) {
        abortrace = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!abortrace && abortrace->getStatus() == RACE_STATUS_RUNNING) {
          ui->goConfirmation("Do you really want to abort the spread job " + abortrace->getName());
        }
      }
      return true;
    case 'z':
      if (hascontents) {
        abortdeleterace = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!abortdeleterace) {
          if (abortdeleterace->getStatus() == RACE_STATUS_RUNNING) {
            ui->goConfirmation("Do you really want to abort the race " + abortdeleterace->getName() + " and delete your own files on all involved sites?");
          }
          else {
            ui->goConfirmation("Do you really want to delete your own files in " + abortdeleterace->getName() + " on all involved sites?");
          }
        }
      }
      return true;
    case 'r':
    case 'R':
      if (hascontents) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!race) {
          global->getEngine()->resetRace(race, ch == 'R');
        }
      }
      return true;
    case 't':
    case 'T':
      if (hascontents) {
        std::shared_ptr<Race> race = global->getEngine()->getRace(table.getElement(table.getSelectionPointer())->getId());
        if (!!race) {
          ui->goTransfersFilterSpreadJob(race->getName());
        }
      }
      return true;
    case '-':
      if (!hascontents) {
        break;
      }
      temphighlightline = table.getElement(table.getSelectionPointer())->getRow();
      ui->redraw();
      return true;
  }
  return false;
}

std::string AllRacesScreen::getLegendText() const {
  return "[Esc/c] Return - [Enter] Details - [Up/Down/Pgup/Pgdn/Home/End] Navigate - [r]eset job - Hard [R]eset job - A[B]ort job - [t]ransfer for job - [z] Abort job and delete own files on all sites";
}

std::string AllRacesScreen::getInfoLabel() const {
  return "ALL SPREAD JOBS";
}

std::string AllRacesScreen::getInfoText() const {
  return "Active: " + std::to_string(engine->currentRaces()) + "  Total: " + std::to_string(engine->allRaces());
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

  msotb = mso.addTextButton(y, 1, "release", release);
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
    case RACE_STATUS_RUNNING:
      status = "running";
      break;
    case RACE_STATUS_DONE:
      status = "done";
      break;
    case RACE_STATUS_ABORTED:
      status = "aborted";
      break;
    case RACE_STATUS_TIMEOUT:
      status = "timeout";
      break;
  }
  addRaceTableRow(y, mso, race->getId(), true, race->getTimeStamp(), timespent, race->getSection(),
                  race->getName(), size, worst, avg, best, status, done,
                  race->getSiteListText());
}
