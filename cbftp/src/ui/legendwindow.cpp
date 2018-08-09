#include "legendwindow.h"

#include "termint.h"
#include "ui.h"
#include "legendprinterkeybinds.h"
#include "../eventlog.h"
#include "../globalcontext.h"

LegendWindow::LegendWindow(Ui * ui, WINDOW * window, int row, int col) {
  this->ui = ui;
  this->window = window;
  split = false;
  latestid = 0;
  latestcount = 8;
  init(row, col);
}

void LegendWindow::redraw() {
  ui->erase(window);
  if (!!mainlegendprinter) {
    mainlegendprinter->setColumns(col);
  }
  std::list<std::shared_ptr<LegendPrinter> >::iterator it;
  for (it = templegendprinters.begin(); it != templegendprinters.end(); it++) {
    if (!!(*it)) {
      (*it)->setColumns(col);
    }
  }
  latestcount = 8;
  staticcount = 0;
  offset = 0;
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
    if (!templegendprinters.empty()) {
      return;
    }
    latestcount = 0;
    latesttext = global->getEventLog()->getLatest();
    for (unsigned int printpos = 4; printpos < col - 4; printpos++) {
      ui->printChar(window, 1, printpos, ' ');
    }
    ui->printStr(window, 1, 4, "EVENT: " + latesttext, col - 4 - 4, false);
    return;
  }
  if (latestcount < 8) { // 2 seconds
    latestcount++;
    return;
  }
  if (!templegendprinters.empty() && !!templegendprinters.front()) {
    if (!templegendprinters.front()->print()) {
      templegendprinters.pop_front();
    }
  }
  else if (!!mainlegendprinter) {
    mainlegendprinter->print();
  }
}

void LegendWindow::setSplit(bool split) {
  this->split = split;
}

void LegendWindow::setMainLegendPrinter(std::shared_ptr<LegendPrinter> printer) {
  mainlegendprinter = printer;
}

void LegendWindow::addTempLegendPrinter(std::shared_ptr<LegendPrinter> printer) {
  templegendprinters.push_back(printer);
}

void LegendWindow::clearTempLegendPrinters() {
  templegendprinters.clear();
}
