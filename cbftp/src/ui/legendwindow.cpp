#include "legendwindow.h"

#include "termint.h"
#include "ui.h"
#include "../eventlog.h"
#include "../globalcontext.h"

extern GlobalContext * global;

LegendWindow::LegendWindow(Ui * ui, WINDOW * window, int row, int col) {
  this->ui = ui;
  this->window = window;
  split = false;
  text = "";
  latestid = 0;
  latestcount = 8;
  init(row, col);
}

void LegendWindow::redraw() {
  ui->erase(window);
  latestcount = 8;
  currentpos = 0;
  ui->printChar(window, 0, 1, BOX_CORNER_TL);
  ui->printChar(window, 1, 0, BOX_HLINE);
  ui->printChar(window, 1, 1, BOX_CORNER_BR);
  ui->printChar(window, 1, col - 1, BOX_HLINE);
  ui->printChar(window, 1, col - 2, BOX_CORNER_BL);
  ui->printChar(window, 0, col - 2, BOX_CORNER_TR);
  for (unsigned int i = 2; i < col - 2; i++) {
    ui->printChar(window, 0, i, BOX_HLINE);
  }
  if (split) {
    ui->printChar(window, 0, col / 2, BOX_HLINE_TOP);
  }
  update();
}

void LegendWindow::update() {
  if (global->getEventLog()->getLatestId() != latestid) {
    latestid = global->getEventLog()->getLatestId();
    latestcount = 0;
    latesttext = global->getEventLog()->getLatest();
    for (unsigned int printpos = 4; printpos < col - 4; printpos++) {
      ui->printChar(window, 1, printpos, ' ');
    }
    ui->printStr(window, 1, 4, "EVENT: " + latesttext, col - 4 - 4, false);
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
        ui->printChar(window, 1, printpos, text[printpos - internalpos]);
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

void LegendWindow::setSplit(bool split) {
  this->split = split;
}
