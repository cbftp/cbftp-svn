#include "eventlogscreen.h"

#include "../../eventlog.h"
#include "../../rawbuffer.h"
#include "../../globalcontext.h"

#include "../uicommunicator.h"
#include "../termint.h"

extern GlobalContext * global;

EventLogScreen::EventLogScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  uicommunicator->expectBackendPush();
  this->rawbuf = global->getEventLog()->getRawBuffer();
  rawbuf->uiWatching(true);
  readfromcopy = false;
  copyreadpos = 0;
  init(window, row, col);
}

void EventLogScreen::redraw() {
  update();
}

void EventLogScreen::update() {
  werase(window);
  if (!readfromcopy) {
    unsigned int numlinestoprint = rawbuf->getSize() < row ? rawbuf->getSize() : row;
    for (unsigned int i = 0; i < numlinestoprint; i++) {
      TermInt::printStr(window, i, 0, rawbuf->getLine(numlinestoprint - i - 1));
    }
  }
  else {
    unsigned int numlinestoprint = copysize < row ? copysize : row;
    for (unsigned int i = 0; i < numlinestoprint; i++) {
      TermInt::printStr(window, i, 0, rawbuf->getLineCopy(numlinestoprint - i - 1 + copyreadpos));
    }
  }
}

void EventLogScreen::keyPressed(unsigned int ch) {
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
      uicommunicator->newCommand("update");
      break;
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
      uicommunicator->newCommand("update");
      break;
    case 10:
    case 27: // esc
      rawbuf->uiWatching(false);
      uicommunicator->newCommand("return");
      break;
  }
}

std::string EventLogScreen::getLegendText() {
  return "[Pgup] Scroll up - [Pgdn] Scroll down - [ESC/Enter] Return";
}

std::string EventLogScreen::getInfoLabel() {
  return "EVENT LOG";
}
