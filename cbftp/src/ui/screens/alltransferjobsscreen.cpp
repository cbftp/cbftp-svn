#include "alltransferjobsscreen.h"

#include "transferjobstatusscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../menuselectoptiontextbutton.h"
#include "../misc.h"

#include "../../globalcontext.h"
#include "../../util.h"
#include "../../engine.h"
#include "../../transferjob.h"

AllTransferJobsScreen::AllTransferJobsScreen(Ui * ui) {
  this->ui = ui;
}

void AllTransferJobsScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  hascontents = false;
  currentviewspan = 0;
  ypos = 1;
  temphighlightline = -1;
  engine = global->getEngine();
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void AllTransferJobsScreen::redraw() {
  ui->erase();
  unsigned int y = 0;
  unsigned int totallistsize = engine->allTransferJobs() + 1;
  table.reset();
  adaptViewSpan(currentviewspan, row, ypos, totallistsize);

  if (!currentviewspan) {
    addJobTableHeader(y, table, "NAME");
    y++;
  }
  unsigned int pos = 1;
  for (std::list<std::shared_ptr<TransferJob> >::const_iterator it = --engine->getTransferJobsEnd(); it != --engine->getTransferJobsBegin() && y < row; it--) {
    if (pos >= currentviewspan) {
      addJobDetails(y++, table, *it);
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
  printSlider(ui, row, col - 1, totallistsize, currentviewspan);
}

void AllTransferJobsScreen::update() {
  redraw();
}

void AllTransferJobsScreen::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    global->getEngine()->abortTransferJob(abortjob);
  }
  ui->redraw();
}

bool AllTransferJobsScreen::keyPressed(unsigned int ch) {
  if (temphighlightline != -1) {
    temphighlightline = -1;
    ui->redraw();
    if (ch == '-') {
      return true;
    }
  }
  switch (ch) {
    case KEY_UP:
      if (hascontents && ypos > 1) {
        --ypos;
        table.goUp();
        ui->update();
      }
      else if (ypos == 1) {
        currentviewspan = 0;
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (hascontents && ypos < engine->allTransferJobs()) {
        ++ypos;
        table.goDown();
        ui->update();
      }
      return true;
    case KEY_NPAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows; i++) {
        if (ypos >= engine->allTransferJobs()) {
          break;
        }
        ypos++;
        table.goDown();
      }
      ui->update();
      return true;
    }
    case KEY_PPAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows; i++) {
        if (ypos == 1) {
          currentviewspan = 0;
          break;
        }
        ypos--;
        table.goUp();
      }
      ui->update();
      return true;
    }
    case KEY_HOME:
      ypos = 1;
      currentviewspan = 0;
      ui->update();
      return true;
    case KEY_END:
      ypos = engine->allTransferJobs();
      ui->update();
      return true;
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
    case 10:
      if (hascontents) {
        std::shared_ptr<MenuSelectOptionTextButton> msotb =
            std::static_pointer_cast<MenuSelectOptionTextButton>(table.getElement(table.getSelectionPointer()));
        ui->goTransferJobStatus(msotb->getId());
      }
      return true;
    case 'B':
      if (hascontents) {
        abortjob = global->getEngine()->getTransferJob(table.getElement(table.getSelectionPointer())->getId());
        if (!!abortjob && !abortjob->isDone()) {
          ui->goConfirmation("Do you really want to abort the transfer job " + abortjob->getName());
        }
      }
      return true;
    case 't':
    case 'T':
      if (hascontents) {
        std::shared_ptr<TransferJob> tj = global->getEngine()->getTransferJob(table.getElement(table.getSelectionPointer())->getId());
        if (!!tj) {
          ui->goTransfersFilterTransferJob(tj->getName());
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

std::string AllTransferJobsScreen::getLegendText() const {
  return "[Esc/c] Return - [Enter] Details - [Up/Down] Navigate - a[B]ort job - [t]ransfers for job";
}

std::string AllTransferJobsScreen::getInfoLabel() const {
  return "ALL TRANSFER JOBS";
}

std::string AllTransferJobsScreen::getInfoText() const {
  return "Active: " + std::to_string(engine->currentTransferJobs()) + "  Total: " + std::to_string(engine->allTransferJobs());
}

void AllTransferJobsScreen::addJobTableRow(unsigned int y, MenuSelectOption & mso, unsigned int id, bool selectable,
    const std::string & queuetime, const std::string & starttime, const std::string & timespent, const std::string & type,
    const std::string & name, const std::string & route, const std::string & sizeprogress, const std::string & filesprogress,
    const std::string & timeremaining, const std::string & speed, const std::string & progress) {
  std::shared_ptr<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "queuetime", queuetime);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "starttime", starttime);
  msotb->setSelectable(false);
  msal->addElement(msotb, 4, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timespent", timespent);
  msotb->setSelectable(false);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButton(y, 1, "type", type);
  msotb->setSelectable(false);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButton(y, 1, "name", name);
  msotb->setSelectable(selectable);
  msotb->setId(id);
  msal->addElement(msotb, 11, 0, RESIZE_CUTEND, true);

  msotb = mso.addTextButton(y, 1, "route", route);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "sizeprogress", sizeprogress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 9, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "filesprogress", filesprogress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 1, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timeremaining", timeremaining);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "speed", speed);
  msotb->setSelectable(false);
  msal->addElement(msotb, 7, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "progress", progress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 10, RESIZE_REMOVE);
}

void AllTransferJobsScreen::addJobTableHeader(unsigned int y, MenuSelectOption & mso, const std::string & name) {
  addJobTableRow(y, mso, -1, false, "QUEUED", "STARTED", "USE", "TYPE", name, "ROUTE", "SIZE", "FILES", "LEFT", "SPEED", "DONE");
}

void AllTransferJobsScreen::addJobDetails(unsigned int y, MenuSelectOption & mso, std::shared_ptr<TransferJob> tj) {
  std::string timespent = util::simpleTimeFormat(tj->timeSpent());
  bool running = tj->getStatus() == TRANSFERJOB_RUNNING;
  bool started = tj->getStatus() != TRANSFERJOB_QUEUED;
  std::string timeremaining = running ? util::simpleTimeFormat(tj->timeRemaining()) : "-";
  std::string route = TransferJobStatusScreen::getRoute(tj);
  std::string sizeprogress = util::parseSize(tj->sizeProgress()) +
                             " / " + util::parseSize(tj->totalSize());
  std::string filesprogress = std::to_string(tj->filesProgress()) + "/" +
                              std::to_string(tj->filesTotal());
  std::string speed = started ? util::parseSize(tj->getSpeed() * SIZEPOWER) + "/s" : "-";
  std::string progress = std::to_string(tj->getProgress()) + "%";
  std::string status;
  switch (tj->getStatus()) {
    case TRANSFERJOB_QUEUED:
      status = "queue";
      break;
    case TRANSFERJOB_RUNNING:
      status = progress;
      break;
    case TRANSFERJOB_DONE:
      status = "done";
      break;
    case TRANSFERJOB_ABORTED:
      status = "abor";
      break;
  }
  std::string type = "FXP";
  switch (tj->getType()) {
    case TRANSFERJOB_DOWNLOAD:
      type = "DL";
      break;
    case TRANSFERJOB_UPLOAD:
      type = "UL";
      break;
  }
  addJobTableRow(y, mso, tj->getId(), true, tj->timeQueued(), tj->timeStarted(), timespent, type, tj->getSrcFileName(), route, sizeprogress, filesprogress, timeremaining, speed, status);
}
