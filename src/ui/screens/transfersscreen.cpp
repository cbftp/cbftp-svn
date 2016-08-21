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
}

TransfersScreen::~TransfersScreen() {

}

void TransfersScreen::initialize(unsigned int row, unsigned int col) {
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
  progressmap.clear();
  adaptViewSpan(currentviewspan, listspan, ypos, totallistsize);

  addTransferTableHeader(y++, table);

  unsigned int pos = 0;
  for (std::list<Pointer<TransferStatus> >::const_iterator it = tm->ongoingTransfersBegin(); it != tm->ongoingTransfersEnd() && y < row; it++) {
    if (pos >= currentviewspan) {
      progressmap[addTransferDetails(y++, table, *it)] = (*it)->getProgress();
      if (pos == ypos) {
        table.enterFocusFrom(2);
      }
    }
    ++pos;
  }
  for (std::list<Pointer<TransferStatus> >::const_iterator it = tm->finishedTransfersBegin(); it != tm->finishedTransfersEnd() && y < row; it++) {
    if (pos >= currentviewspan) {
      progressmap[addTransferDetails(y++, table, *it)] = (*it)->getProgress();
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
        std::map<Pointer<MenuSelectOptionElement>, int>::iterator it = progressmap.find(re);
        if (it != progressmap.end()) {
          progresspercent = it->second;
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
    case 'c':
    case 27: // esc
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string TransfersScreen::getLegendText() const {
  return "[Enter] return";
}

std::string TransfersScreen::getInfoLabel() const {
  return "TRANSFERS";
}

void TransfersScreen::addTransferTableHeader(unsigned int y, MenuSelectOption & mso) {
  addTransferTableRow(y, mso, false, "STARTED", "USE", "ROUTE", "PATH", "TRANSFERRED", "FILENAME", "LEFT", "SPEED", "DONE");
}

Pointer<MenuSelectOptionElement> TransfersScreen::addTransferDetails(unsigned int y, MenuSelectOption & mso, Pointer<TransferStatus> ts) {
  std::string route = ts->getSource() + " -> " + ts->getTarget();
  std::string speed = util::parseSize(ts->getSpeed() * SIZEPOWER) + "/s";
  std::string timespent = util::simpleTimeFormat(ts->getTimeSpent());
  int progresspercent = ts->getProgress();
  std::string progress;
  switch (ts->getState()) {
    case TRANSFERSTATUS_STATE_IN_PROGRESS:
      progress = util::int2Str(progresspercent) + "%";
      break;
    case TRANSFERSTATUS_STATE_FAILED:
      progress = "fail";
      break;
    case TRANSFERSTATUS_STATE_SUCCESSFUL:
      progress = "done";
      break;
  }
  std::string timeremaining = util::simpleTimeFormat(ts->getTimeRemaining());
  std::string transferred = util::parseSize(ts->targetSize()) + " / " +
      util::parseSize(ts->sourceSize());
  std::string path = ts->getSourcePath() + " -> " + ts->getTargetPath();

  return addTransferTableRow(y, mso, true, ts->getTimestamp(), timespent, route, path, transferred,
      ts->getFile(), timeremaining, speed, progress);
}

Pointer<MenuSelectOptionElement> TransfersScreen::addTransferTableRow(unsigned int y, MenuSelectOption & mso, bool selectable,
    const std::string & timestamp, const std::string & timespent, const std::string & route,
    const std::string & path, const std::string & transferred, const std::string & filename,
    const std::string & timeremaining, const std::string & speed, const std::string & progress)
{
  Pointer<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;

  msotb = mso.addTextButtonNoContent(y, 1, "timestamp", timestamp);
  msotb->setSelectable(false);
  msal->addElement(msotb, 3, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 1, "timespent", timespent);
  msotb->setSelectable(false);
  msal->addElement(msotb, 9, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 30, "route", route);
  msotb->setSelectable(false);
  msal->addElement(msotb, 8, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 10, "path", path);
  msotb->setSelectable(false);
  msal->addElement(msotb, 1, RESIZE_WITHDOTS);

  msotb = mso.addTextButtonNoContent(y, 10, "transferred", transferred);
  msotb->setSelectable(false);
  msal->addElement(msotb, 4, RESIZE_CUTEND);
  Pointer<MenuSelectOptionElement> progresselem = msotb;

  msotb = mso.addTextButtonNoContent(y, 10, "filename", filename);
  msotb->setSelectable(selectable);
  msal->addElement(msotb, 2, RESIZE_WITHLAST3, true);

  msotb = mso.addTextButtonNoContent(y, 60, "remaining", timeremaining);
  msotb->setSelectable(false);
  msal->addElement(msotb, 5, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 40, "speed", speed);
  msotb->setSelectable(false);
  msal->addElement(msotb, 6, RESIZE_REMOVE);

  msotb = mso.addTextButtonNoContent(y, 50, "progress", progress);
  msotb->setSelectable(false);
  msal->addElement(msotb, 7, RESIZE_REMOVE);
  return progresselem;
}
