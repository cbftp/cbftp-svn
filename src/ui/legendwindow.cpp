#include "legendwindow.h"

#include "termint.h"
#include "../eventlog.h"
#include "../globalcontext.h"

extern GlobalContext * global;

LegendWindow::LegendWindow(WINDOW * window, int row, int col) {
  text = "";
  latestid = 0;
  latestcount = 8;
  init(window, row, col);
}

void LegendWindow::redraw() {
  werase(window);
  currentpos = 0;
  TermInt::printChar(window, 0, 1, 4194412);
  TermInt::printChar(window, 1, 0, 4194417);
  TermInt::printChar(window, 1, 1, 4194410);
  TermInt::printChar(window, 1, col - 1, 4194417);
  TermInt::printChar(window, 1, col - 2, 4194413);
  TermInt::printChar(window, 0, col - 2, 4194411);
  for (unsigned int i = 2; i < col - 2; i++) {
    TermInt::printChar(window, 0, i, 4194417);
  }
  update();
}

void LegendWindow::update() {
  if (global->getEventLog()->getLatestId() != latestid) {
    latestid = global->getEventLog()->getLatestId();
    latestcount = 0;
    latesttext = global->getEventLog()->getLatest();
    for (unsigned int printpos = 4; printpos < col - 4; printpos++) {
      TermInt::printChar(window, 1, printpos, ' ');
    }
    TermInt::printStr(window, 1, 4, "EVENT: " + latesttext, col - 4 - 4);
    return;
  }
  if (latestcount < 8) {
    latestcount++;
    return;
  }
  unsigned int printpos = 4;
  unsigned int textlen = text.length();
  if (textlen > 0) {
    unsigned int internalpos = printpos - currentpos++;
    if (currentpos >= textlen) currentpos = 0;
    while (printpos < col - 4) {
      while (printpos - internalpos < textlen && printpos < col - 4) {
        TermInt::printChar(window, 1, printpos, text[printpos - internalpos]);
        ++printpos;
      }
      internalpos = printpos;
    }
  }
}

void LegendWindow::setText(std::string text) {
  this->text = text + "  ::  ";
  currentpos = 0;
  update();

}
