#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"

#include "../../globalcontext.h"
#include "../../transferstatus.h"
#include "../../transfermanager.h"
#include "../../util.h"

extern GlobalContext * global;

TransfersScreen::TransfersScreen(Ui * ui) {
  this->ui = ui;
  tm = global->getTransferManager();
}

TransfersScreen::~TransfersScreen() {

}

void TransfersScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void TransfersScreen::redraw() {
  ui->erase();
  int y = 0;
  table.clear();
  progressmap.clear();
  Pointer<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;
  msotb = table.addTextButtonNoContent(y, 1, "timestamp", "STARTED");
  msal->addElement(msotb, 3, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 4, "timespent", "USE");
  msal->addElement(msotb, 9, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 2, "route", "ROUTE");
  msal->addElement(msotb, 8, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 10, "path", "PATH");
  msal->addElement(msotb, 1, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 5, "transferred", "TRANSFERRED");
  msal->addElement(msotb, 4, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 3, "filename", "FILENAME");
  msal->addElement(msotb, 2, RESIZE_CUTEND, true);
  msotb = table.addTextButtonNoContent(y, 6, "remaining", "LEFT");
  msal->addElement(msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 7, "speed", "SPEED");
  msal->addElement(msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 8, "progress", "DONE");
  msal->addElement(msotb, 7, RESIZE_REMOVE);

  y++;
  for (std::list<Pointer<TransferStatus> >::const_iterator it = tm->ongoingTransfersBegin(); it != tm->ongoingTransfersEnd(); it++) {
    addTransferDetails(y++, table, *it);
  }
  for (std::list<Pointer<TransferStatus> >::const_iterator it = tm->finishedTransfersBegin(); it != tm->finishedTransfersEnd(); it++) {
    addTransferDetails(y++, table, *it);
  }
  table.adjustLines(col - 3);
  bool highlight;
  for (unsigned int i = 0; i < table.size(); i++) {
    Pointer<ResizableElement> re = table.getElement(i);
    highlight = false;
    if (table.getSelectionPointer() == i) {
      //highlight = true; // later problem
    }
    if (re->isVisible()) {
      if (re->getIdentifier() == "filename") {
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
}

void TransfersScreen::update() {
  redraw();
}

bool TransfersScreen::keyPressed(unsigned int ch) {
  switch (ch) {
    case 'c':
    case 27: // esc
    case 10:
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

void TransfersScreen::addTransferDetails(unsigned int y, MenuSelectOption & mso, Pointer<TransferStatus> ts) {
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
  Pointer<MenuSelectAdjustableLine> msal = mso.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;
  msotb = mso.addTextButtonNoContent(y, 1, "timestamp", ts->getTimestamp());
  msal->addElement(msotb, 3, RESIZE_REMOVE);
  msotb = mso.addTextButtonNoContent(y, 1, "timespent", timespent);
  msal->addElement(msotb, 9, RESIZE_REMOVE);
  msotb = mso.addTextButtonNoContent(y, 30, "route", route);
  msal->addElement(msotb, 8, RESIZE_REMOVE);
  msotb = mso.addTextButtonNoContent(y, 10, "path", path);
  msal->addElement(msotb, 1, RESIZE_WITHDOTS);
  msotb = mso.addTextButtonNoContent(y, 10, "transferred", transferred);
  msal->addElement(msotb, 4, RESIZE_CUTEND);
  msotb = mso.addTextButtonNoContent(y, 10, "filename", ts->getFile());
  progressmap[msotb] = progresspercent;
  msal->addElement(msotb, 2, RESIZE_WITHLAST3, true);
  msotb = mso.addTextButtonNoContent(y, 60, "remaining", timeremaining);
  msal->addElement(msotb, 5, RESIZE_REMOVE);
  msotb = mso.addTextButtonNoContent(y, 40, "speed", speed);
  msal->addElement(msotb, 6, RESIZE_REMOVE);
  msotb = mso.addTextButtonNoContent(y, 50, "progress", progress);
  msal->addElement(msotb, 7, RESIZE_REMOVE);

}
