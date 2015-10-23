#include "alltransferjobsscreen.h"

#include "transferjobstatusscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../menuselectoptiontextbutton.h"

#include "../../globalcontext.h"
#include "../../util.h"
#include "../../engine.h"
#include "../../transferjob.h"

extern GlobalContext * global;

AllTransferJobsScreen::AllTransferJobsScreen(Ui * ui) {
  this->ui = ui;
}

void AllTransferJobsScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  hascontents = false;
  engine = global->getEngine();
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void AllTransferJobsScreen::redraw() {
  ui->erase();
  int y = 0;
  table.clear();
  addJobTableHeader(y, table, "NAME");
  y++;
  for (std::list<Pointer<TransferJob> >::const_iterator it = --engine->getTransferJobsEnd(); it != --engine->getTransferJobsBegin(); it--) {
    addJobDetails(y++, table, *it);
  }
  table.checkPointer();
  hascontents = table.linesSize() > 1;
  table.adjustLines(col - 3);
  bool highlight;
  for (unsigned int i = 0; i < table.size(); i++) {
    Pointer<ResizableElement> re = table.getElement(i);
    highlight = false;
    if (table.getSelectionPointer() == i && hascontents) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
    }
  }
}

void AllTransferJobsScreen::update() {
  redraw();
}

void AllTransferJobsScreen::keyPressed(unsigned int ch) {
  switch (ch) {
    case KEY_UP:
      if (hascontents && table.goUp()) {
        ui->update();
      }
      break;
    case KEY_DOWN:
      if (hascontents && table.goDown()) {
        ui->update();
      }
      break;
    case 'c':
    case 27: // esc
      ui->returnToLast();
      break;
    case 10:
      if (hascontents) {
        Pointer<MenuSelectOptionTextButton> msotb =
            table.getElement(table.getSelectionPointer());
        ui->goTransferJobStatus(msotb->getId());
      }
      break;
  }
}

std::string AllTransferJobsScreen::getLegendText() const {
  return "[Esc/c] Return - [Enter] Details - [Up/Down] Navigate";
}

std::string AllTransferJobsScreen::getInfoLabel() const {
  return "ALL TRANSFER JOBS";
}

std::string AllTransferJobsScreen::getInfoText() const {
  return "Active: " + util::int2Str(engine->currentTransferJobs()) + "  Total: " + util::int2Str(engine->allTransferJobs());
}

void AllTransferJobsScreen::addJobTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id, bool selectable,
    std::string timestamp, std::string timespent, std::string type, std::string name,
    std::string route, std::string sizeprogress, std::string filesprogress,
    std::string remaining, std::string speed, std::string progress) {
  Pointer<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "timestamp", timestamp);
  msotb->setSelectable(false);
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timespent", timespent);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButton(y, 1, "type", type);
  msotb->setSelectable(false);
  msal->addElement(msotb, 7, RESIZE_REMOVE);

  msotb = mso.addTextButton(y, 1, "name", name);
  if (!selectable) {
    msotb->setSelectable(false);
  }
  msotb->setId(id);
  msal->addElement(msotb, 10, RESIZE_CUTEND, true);

  msotb = mso.addTextButton(y, 1, "route", route);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "sizeprogress", sizeprogress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "filesprogress", filesprogress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 1, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "remaining", remaining);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "speed", speed);
  msotb->setSelectable(false);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "progress", progress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 9, RESIZE_REMOVE);
}

void AllTransferJobsScreen::addJobTableHeader(unsigned int y, MenuSelectOption & mso, std::string name) {
  addJobTableRow(y, mso, -1, false, "STARTED", "USE", "TYPE", name, "ROUTE", "SIZE", "FILES", "LEFT", "SPEED", "DONE");
}

void AllTransferJobsScreen::addJobDetails(unsigned int y, MenuSelectOption & mso, Pointer<TransferJob> tj) {
  std::string timespent = util::simpleTimeFormat(tj->timeSpent());
  bool aborted = tj->isAborted();
  std::string timeremaining = aborted ? "-" : util::simpleTimeFormat(tj->timeRemaining());
  std::string route = TransferJobStatusScreen::getRoute(tj);
  std::string sizeprogress = util::parseSize(tj->sizeProgress()) +
                             " / " + util::parseSize(tj->totalSize());
  std::string filesprogress = util::int2Str(tj->filesProgress()) + "/" +
                              util::int2Str(tj->filesTotal());
  std::string speed = util::parseSize(tj->getSpeed() * SIZEPOWER) + "/s";
  std::string progress = util::int2Str(tj->getProgress()) + "%";
  std::string status = tj->isDone() ? (aborted ? "abor" : "done") : progress;
  std::string type = "FXP";
  switch (tj->getType()) {
    case TRANSFERJOB_DOWNLOAD:
      type = "DL";
      break;
    case TRANSFERJOB_UPLOAD:
      type = "UL";
      break;
    case TRANSFERJOB_DOWNLOAD_FILE:
      type = "DLF";
      break;
    case TRANSFERJOB_UPLOAD_FILE:
      type = "ULF";
      break;
    case TRANSFERJOB_FXP_FILE:
      type = "FXPF";
      break;
  }
  addJobTableRow(y, mso, tj->getId(), true, tj->timeStarted(), timespent, type, tj->getSrcFileName(), route, sizeprogress, filesprogress, timeremaining, speed, status);
}
