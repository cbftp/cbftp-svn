#include "alltransferjobsscreen.h"

#include "../ui.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptionelement.h"
#include "../resizableelement.h"

#include "../../globalcontext.h"

//extern GlobalContext * global;

AllTransferJobsScreen::AllTransferJobsScreen(Ui * ui) {
  this->ui = ui;
}

void AllTransferJobsScreen::initialize(unsigned int row, unsigned int col) {
  autoupdate = true;
  table.reset();
  table.enterFocusFrom(0);
  init(row, col);
}

void AllTransferJobsScreen::redraw() {
  ui->erase();
  int y = 0;
  table.clear();
  MenuSelectAdjustableLine * msal = table.addAdjustableLine();
  MenuSelectOptionTextButton * msotb;
  msotb = table.addTextButtonNoContent(y, 1, "timestamp", "STARTED");
  msal->addElement((ResizableElement *)msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 1, "type", "TYPE");
  msal->addElement((ResizableElement *)msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 4, "name", "NAME");
  msal->addElement((ResizableElement *)msotb, 9, RESIZE_CUTEND, true);
  msotb = table.addTextButtonNoContent(y, 2, "route", "ROUTE");
  msal->addElement((ResizableElement *)msotb, 2, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 2, "speed", "SPEED");
  msal->addElement((ResizableElement *)msotb, 3, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 8, "progress", "DONE");
  msal->addElement((ResizableElement *)msotb, 8, RESIZE_REMOVE);
  y++;
  table.adjustLines(col - 3);
  bool highlight;
  for (unsigned int i = 0; i < table.size(); i++) {
    ResizableElement * re = (ResizableElement *) table.getElement(i);
    highlight = false;
    if (table.getSelectionPointer() == i) {
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
    case 'c':
    case 27: // esc
    case 10:
      ui->returnToLast();
      break;
  }
}

std::string AllTransferJobsScreen::getLegendText() const {
  return "[Enter] return";
}

std::string AllTransferJobsScreen::getInfoLabel() const {
  return "ALL TRANSFER JOBS";
}
