#include "eventlogscreen.h"

#include "../../eventlog.h"
#include "../../rawbuffer.h"
#include "../../globalcontext.h"

#include "../ui.h"

extern GlobalContext * global;

EventLogScreen::EventLogScreen(Ui * ui) {
  this->ui = ui;
}

void EventLogScreen::initialize(unsigned int row, unsigned int col) {
  expectbackendpush = true;
  this->rawbuf = global->getEventLog()->getRawBuffer();
  rawbuf->uiWatching(true);
  readfromcopy = false;
  copyreadpos = 0;
  init(row, col);
}

void EventLogScreen::redraw() {
  update();
}

void EventLogScreen::update() {
  ui->erase();
  if (!readfromcopy) {
    unsigned int numlinestoprint = rawbuf->getSize() < row ? rawbuf->getSize() : row;
    for (unsigned int i = 0; i < numlinestoprint; i++) {
      std::pair<std::string, std::string> entry = rawbuf->getLine(numlinestoprint - i - 1);
      std::string line = entry.first + " " + entry.second;
      ui->printStr(i, 0, line);
    }
  }
  else {
    unsigned int numlinestoprint = copysize < row ? copysize : row;
    for (unsigned int i = 0; i < numlinestoprint; i++) {
      std::pair<std::string, std::string> entry = rawbuf->getLineCopy(numlinestoprint - i - 1 + copyreadpos);
      std::string line = entry.first + " " + entry.second;
      ui->printStr(i, 0, line);
    }
  }
}

bool EventLogScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case KEY_PPAGE:
      if (!readfromcopy) {
        rawbuf->freezeCopy();
        copyreadpos = 0;
        copysize = rawbuf->getCopySize();
        readfromcopy = true;
      }
      else {
        copyreadpos = copyreadpos + row / 2;
        if (row >= copysize) {
          copyreadpos = 0;
        }
        else if (copyreadpos + row > copysize) {
          copyreadpos = copysize - row;
        }
      }
      ui->update();
      return true;
    case KEY_NPAGE:
      if (readfromcopy) {
        if (copyreadpos == 0) {
          readfromcopy = false;
        }
        else if (copyreadpos < row / 2) {
          copyreadpos = 0;
        }
        else {
          copyreadpos = copyreadpos - row / 2;
        }
      }
      ui->update();
      return true;
    case 10:
    case 27: // esc
      rawbuf->uiWatching(false);
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string EventLogScreen::getLegendText() const {
  return "[Pgup] Scroll up - [Pgdn] Scroll down - [ESC/Enter] Return";
}

std::string EventLogScreen::getInfoLabel() const {
  return "EVENT LOG";
}
