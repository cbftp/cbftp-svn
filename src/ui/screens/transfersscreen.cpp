#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"

#include "../../globalcontext.h"
#include "../../transferstatus.h"
#include "../../transfermanager.h"

extern GlobalContext * global;

TransfersScreen::TransfersScreen(Ui * ui) {
  this->ui = ui;
  tm = global->getTransferManager();
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
  MenuSelectAdjustableLine * msal = table.addAdjustableLine();
  MenuSelectOptionTextButton * msotb;
  msotb = table.addTextButtonNoContent(y, 1, "timestamp", "STARTED");
  msal->addElement((ResizableElement *)msotb, 3, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 4, "timespent", "USE");
  msal->addElement((ResizableElement *)msotb, 9, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 2, "route", "ROUTE");
  msal->addElement((ResizableElement *)msotb, 8, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 10, "path", "PATH");
  msal->addElement((ResizableElement *)msotb, 1, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 5, "transferred", "TRANSFERRED");
  msal->addElement((ResizableElement *)msotb, 4, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 3, "filename", "FILENAME");
  msal->addElement((ResizableElement *)msotb, 2, RESIZE_CUTEND, true);
  msotb = table.addTextButtonNoContent(y, 6, "remaining", "LEFT");
  msal->addElement((ResizableElement *)msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 7, "speed", "SPEED");
  msal->addElement((ResizableElement *)msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 8, "progress", "DONE");
  msal->addElement((ResizableElement *)msotb, 7, RESIZE_REMOVE);

  y++;
  for (std::list<TransferStatus *>::const_iterator it = tm->ongoingTransfersBegin(); it != tm->ongoingTransfersEnd(); it++) {
    addTransferDetails(y++, *it);
  }
  for (std::list<TransferStatus *>::const_iterator it = tm->finishedTransfersBegin(); it != tm->finishedTransfersEnd(); it++) {
    addTransferDetails(y++, *it);
  }
  table.adjustLines(col - 3);
  bool highlight;
  for (unsigned int i = 0; i < table.size(); i++) {
    ResizableElement * re = (ResizableElement *) table.getElement(i);
    highlight = false;
    if (table.getSelectionPointer() == i) {
      //highlight = true; // later problem
    }
    if (re->isVisible()) {
      if (re->getIdentifier() == "filename") {
        int progresspercent = 0;
        std::map<MenuSelectOptionElement *, int>::iterator it = progressmap.find(re);
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

void TransfersScreen::keyPressed(unsigned int ch) {
  switch (ch) {
    case 'c':
    case 27: // esc
    case 10:
      ui->returnToLast();
      break;
  }
}

std::string TransfersScreen::getLegendText() const {
  return "[Enter] return";
}

std::string TransfersScreen::getInfoLabel() const {
  return "TRANSFERS";
}

void TransfersScreen::addTransferDetails(unsigned int y, TransferStatus * ts) {
  std::string route = ts->getSource() + " -> " + ts->getTarget();
  std::string speed = GlobalContext::parseSize(ts->getSpeed() * SIZEPOWER) + "/s";
  std::string timespent = global->int2Str(ts->getTimeSpent()) + "s";
  int progresspercent = ts->getProgress();
  std::string progress = global->int2Str(progresspercent) + "%";
  std::string timeremaining = global->int2Str(ts->getTimeRemaining()) + "s";
  std::string transferred = GlobalContext::parseSize(ts->targetSize()) + " / " +
      GlobalContext::parseSize(ts->sourceSize());
  std::string path = ts->getSourcePath() + " -> " + ts->getTargetPath();
  MenuSelectAdjustableLine * msal = table.addAdjustableLine();
  MenuSelectOptionTextButton * msotb;
  msotb = table.addTextButtonNoContent(y, 1, "timestamp", ts->getTimestamp());
  msal->addElement((ResizableElement *)msotb, 3, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 1, "timespent", timespent);
  msal->addElement((ResizableElement *)msotb, 9, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 30, "route", route);
  msal->addElement((ResizableElement *)msotb, 8, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 10, "path", path);
  msal->addElement((ResizableElement *)msotb, 1, RESIZE_WITHDOTS);
  msotb = table.addTextButtonNoContent(y, 10, "transferred", transferred);
  msal->addElement((ResizableElement *)msotb, 4, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 10, "filename", ts->getFile());
  progressmap[(MenuSelectOptionElement *)msotb] = progresspercent;
  msal->addElement((ResizableElement *)msotb, 2, RESIZE_WITHLAST3, true);
  msotb = table.addTextButtonNoContent(y, 60, "remaining", timeremaining);
  msal->addElement((ResizableElement *)msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 40, "speed", speed);
  msal->addElement((ResizableElement *)msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 50, "progress", progress);
  msal->addElement((ResizableElement *)msotb, 7, RESIZE_REMOVE);

}
