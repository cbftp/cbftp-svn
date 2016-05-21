#include "legendwindow.h"

#include "termint.h"
#include "ui.h"
#include "../eventlog.h"
#include "../globalcontext.h"

LegendWindow::LegendWindow(Ui * ui, WINDOW * window, int row, int col) {
  this->ui = ui;
  this->window = window;
  split = false;
  text = "";
  latestid = 0;
  latestcount = 8;
  staticcount = 0;
  init(row, col);
}

void LegendWindow::redraw() {
  ui->erase(window);
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
  unsigned int printpos = 4;
  if (text.length() > 0) {
    if (ui->legendMode() == LEGEND_SCROLLING) {
      std::string scrollingtext = text + "  ::  ";
      unsigned int textlen = scrollingtext.length();
      unsigned int internalpos = printpos - offset++;
      if (offset >= textlen) offset = 0;
      while (printpos < col - 4) {
        while (printpos - internalpos < textlen && printpos < col - 4) {
          ui->printChar(window, 1, printpos, scrollingtext[printpos - internalpos]);
          ++printpos;
        }
        internalpos = printpos;
      }
    }
    else if (ui->legendMode() == LEGEND_STATIC) {
      if (staticcount++ > 20) { // 5 seconds
        staticcount = 0;
        if (text.length() - offset > col - 8) {
          size_t nextoffset = text.rfind(" - ", offset + col - 8);
          if (nextoffset != std::string::npos) {
            offset = nextoffset + 3;
          }
          else {
            offset += col - 8;
          }
        }
        else if (offset) {
          offset = 0;
        }
      }
      unsigned int textpos = offset;
      while (printpos < col - 4) {
        if (textpos >= text.length()) {
          ui->printChar(window, 1, printpos, ' ');
        }
        else {
          ui->printChar(window, 1, printpos, text[textpos]);
        }
        ++printpos;
        ++textpos;
      }
    }
  }
}

void LegendWindow::setText(std::string text) {
  this->text = text;
  offset = 0;
  staticcount = 0;
  update();
}

void LegendWindow::setSplit(bool split) {
  this->split = split;
}
