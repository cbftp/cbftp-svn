#include "rawdatascreen.h"

RawDataScreen::RawDataScreen(WINDOW * window, UIWindowCommand * windowcommand, int row, int col) {
  this->windowcommand = windowcommand;
  sitename = windowcommand->getArg1();
  threadid = global->str2Int(windowcommand->getArg2());
  SiteThread * sitethread = global->getSiteThreadManager()->getSiteThread(sitename);
  threads = sitethread->getConns()->size();
  this->rawbuf = sitethread->getConn(threadid)->getRawBuffer();
  init(window, row, col);
}

void RawDataScreen::redraw() {
  werase(window);
  int numlinestoprint = rawbuf->getSize() < row ? rawbuf->getSize() : row;
  for (int i = 0; i < numlinestoprint; i++) {
    TermInt::printStr(window, i, 0, rawbuf->getLine(numlinestoprint - i - 1));
  }
}

void RawDataScreen::update() {
  redraw();
}

void RawDataScreen::keyPressed(int ch) {
  switch(ch) {
    case KEY_RIGHT:
      if (threadid + 1 < threads) {
        windowcommand->newCommand("rawdatajump", sitename, global->int2Str(threadid + 1));
      }
      break;
    case KEY_LEFT:
      if (threadid == 0) {
        windowcommand->newCommand("return");
      }
      else {
        windowcommand->newCommand("rawdatajump", sitename, global->int2Str(threadid - 1));
      }
      break;
    case 10:
      windowcommand->newCommand("return");
      break;
  }
}

std::string RawDataScreen::getLegendText() {
  return "[Left] Previous screen - [Right] Next screen - [Enter] Return";
}
