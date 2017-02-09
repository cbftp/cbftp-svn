#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"
#include "../misc.h"

#include "../../globalcontext.h"
#include "../../transferstatus.h"
#include "../../transfermanager.h"
#include "../../util.h"

TransfersScreen::TransfersScreen(Ui * ui) {
  this->ui = ui;
  tm = global->getTransferManager();
  nextid = 0;
}

TransfersScreen::~TransfersScreen() {

}

void TransfersScreen::initialize(unsigned int row, unsigned int col) {
  filtering = false;
  autoupdate = true;
  hascontents = false;
  currentviewspan = 0;
  ypos = 0;
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void TransfersScreen::initialize(unsigned int row, unsigned int col, const TransferFilteringParameters & tfp) {
  filtering = true;
  this->tfp = tfp;
  autoupdate = true;
  hascontents = false;
  currentviewspan = 0;
  ypos = 0;
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void TransfersScreen::redraw() {
  ui->erase();
  unsigned int y = 0;
  unsigned int listspan = row - 1;
  unsigned int totallistsize = tm->ongoingTransfersSize() + tm->finishedTransfersSize();
  table.reset();
  statusmap.clear();
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addTransferTableHeader(y++, table);

  unsigned int pos = 0;
  for (std::list<Pointer<TransferStatus> >::const_iterator it = tm->ongoingTransfersBegin(); it != tm->ongoingTransfersEnd() && y < row; it++) {
    if (pos >= currentviewspan) {
      int id = nextid++;
      addTransferDetails(y++, table, *it, id);
      statusmap[id] = *it;
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
  }
  for (std::list<Pointer<TransferStatus> >::const_iterator it = tm->finishedTransfersBegin(); it != tm->finishedTransfersEnd() && y < row; it++) {
    if (pos >= currentviewspan) {
      int id = nextid++;
      addTransferDetails(y++, table, *it, id);
      statusmap[id] = *it;
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
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
      if (re->getIdentifier() == "transferred") {
        int progresspercent = 0;
        std::map<int, Pointer<TransferStatus> >::iterator it = statusmap.find(re->getId());
        if (it != statusmap.end()) {
          progresspercent = it->second->getProgress();
        }
        std::string labeltext = re->getLabelText();
        int charswithhighlight = labeltext.length() * progresspercent / 100;
        ui->printStr(re->getRow(), re->getCol(), labeltext.substr(0, charswithhighlight), true);
        ui->printStr(re->getRow(), re->getCol() + charswithhighlight, labeltext.substr(charswithhighlight));
      }
      else {
        ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight);
      }
    }
  }
  printSlider(ui, row, 1, col - 1, totallistsize, currentviewspan);
}

void TransfersScreen::update() {
  redraw();
}

bool TransfersScreen::keyPressed(unsigned int ch) {
  switch (ch) {
    case KEY_UP:
      if (hascontents && ypos > 0) {
        --ypos;
        table.goUp();
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (hascontents && ypos < tm->ongoingTransfersSize() + tm->finishedTransfersSize() - 1) {
        ++ypos;
        table.goDown();
        ui->update();
      }
      return true;
    case KEY_NPAGE: {
      unsigned int pagerows = (unsigned int) row * 0.6;
      for (unsigned int i = 0; i < pagerows && ypos < tm->ongoingTransfersSize() + tm->finishedTransfersSize() - 1; i++) {
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
      ypos = tm->ongoingTransfersSize() + tm->finishedTransfersSize() - 1;
      ui->update();
      return true;
    case 10:
     if (hascontents) {
       Pointer<MenuSelectOptionTextButton> elem =
           table.getElement(table.getSelectionPointer());
       std::map<int, Pointer<TransferStatus> >::iterator it = statusmap.find(elem->getId());
       if (it != statusmap.end()) {
         ui->goTransferStatus(it->second);
       }
     }
     return true;
    case 'f':
      if (!filtering) {
        ui->goTransfersFiltering();
      }
      else {
        filtering = false;
        ui->redraw();
      }
      return true;
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string TransfersScreen::getLegendText() const {
  return "[Esc/c] Return - [Up/Down] Navigate - [Enter] Details";
}

std::string TransfersScreen::getInfoLabel() const {
  return "TRANSFERS";
}

void TransfersScreen::addTransferTableHeader(unsigned int y, MenuSelectOption & mso) {
  addTransferTableRow(y, mso, false, "STARTED", "USE", "ROUTE", "PATH", "TRANSFERRED", "FILENAME", "LEFT", "SPEED", "DONE", -1);
}

void TransfersScreen::addTransferDetails(unsigned int y, MenuSelectOption & mso, Pointer<TransferStatus> ts, int id) {
  TransferDetails td = formatTransferDetails(ts);
  addTransferTableRow(y, mso, true, ts->getTimestamp(), td.timespent, td.route, td.path, td.transferred,
      ts->getFile(), td.timeremaining, td.speed, td.progress, id);
}

TransferDetails TransfersScreen::formatTransferDetails(Pointer<TransferStatus> & ts) {
  TransferDetails td;
  td.route = ts->getSource() + " -> " + ts->getTarget();
  td.path = ts->getSourcePath().toString() + " -> " + ts->getTargetPath().toString();
  td.speed = util::parseSize(ts->getSpeed() * SIZEPOWER) + "/s";
  td.timespent = util::simpleTimeFormat(ts->getTimeSpent());
  td.timeremaining = "-";
  td.transferred = util::parseSize(ts->targetSize());
  switch (ts->getState()) {
    case TRANSFERSTATUS_STATE_IN_PROGRESS: {
      int progresspercent = ts->getProgress();
      td.progress = util::int2Str(progresspercent) + "%";
      int timeremainingnum = ts->getTimeRemaining();
      if (timeremainingnum < 0) {
        td.timeremaining = "?";
      }
      else {
        td.timeremaining = util::simpleTimeFormat(timeremainingnum);
      }
      break;
    }
    case TRANSFERSTATUS_STATE_FAILED:
      td.speed = "-";
      td.transferred = "-";
      td.progress = "fail";
      break;
    case TRANSFERSTATUS_STATE_SUCCESSFUL:
      td.progress = "done";
      break;
    case TRANSFERSTATUS_STATE_DUPE:
      td.speed = "-";
      td.transferred = "-";
      td.progress = "dupe";
      break;
  }
  td.transferred += " / " + util::parseSize(ts->sourceSize());
  return td;
}

void TransfersScreen::addTransferTableRow(unsigned int y, MenuSelectOption & mso, bool selectable,
    const std::string & timestamp, const std::string & timespent, const std::string & route,
    const std::string & path, const std::string & transferred, const std::string & filename,
    const std::string & timeremaining, const std::string & speed, const std::string & progress, int id)
{
  Pointer<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "timestamp", timestamp);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timespent", timespent);
  msotb->setSelectable(false);
  msal->addElement(msotb, 9, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 30, "route", route);
  msotb->setSelectable(false);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 10, "path", path);
  msotb->setSelectable(false);
  msal->addElement(msotb, 0, RESIZE_WITHDOTS);

  msotb = mso.addTextButtonNoContent(y, 10, "transferred", transferred);
  msotb->setSelectable(false);
  msotb->setId(id);
  msal->addElement(msotb, 5, RESIZE_CUTEND);

  msotb = mso.addTextButtonNoContent(y, 10, "filename", filename);
  msotb->setSelectable(selectable);
  msotb->setId(id);
  msal->addElement(msotb, 4, 1, RESIZE_WITHLAST3, true);

  msotb = mso.addTextButtonNoContent(y, 60, "remaining", timeremaining);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 40, "speed", speed);
  msotb->setSelectable(false);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 50, "progress", progress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 7, RESIZE_REMOVE);
}
