#include "rawdatascreen.h"

RawDataScreen::RawDataScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  sitename = uicommunicator->getArg1();
  threadid = global->str2Int(uicommunicator->getArg2());
  uicommunicator->expectBackendPush();
  SiteThread * sitethread = global->getSiteThreadManager()->getSiteThread(sitename);
  threads = sitethread->getConns()->size();
  this->rawbuf = sitethread->getConn(threadid)->getRawBuffer();
  rawbuf->uiWatching(true);
  readfromcopy = false;
  copyreadpos = 0;
  init(window, row, col);
}

void RawDataScreen::redraw() {
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

void RawDataScreen::update() {
  redraw();
}

void RawDataScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case KEY_RIGHT:
      if (threadid + 1 < threads) {
        rawbuf->uiWatching(false);
        uicommunicator->newCommand("rawdatajump", sitename, global->int2Str(threadid + 1));
      }
      break;
    case KEY_LEFT:
      if (threadid == 0) {
        rawbuf->uiWatching(false);
        uicommunicator->newCommand("return");
      }
      else {
        rawbuf->uiWatching(false);
        uicommunicator->newCommand("rawdatajump", sitename, global->int2Str(threadid - 1));
      }
      break;
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
      rawbuf->uiWatching(false);
      uicommunicator->newCommand("return");
      break;
  }
}

std::string RawDataScreen::getLegendText() {
  return "[Left] Previous screen - [Right] Next screen - [Enter] Return - [Pgup] Scroll up - [Pgdn] Scroll down";
}
