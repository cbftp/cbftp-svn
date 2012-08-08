#include "rawdatascreen.h"

RawDataScreen::RawDataScreen(WINDOW * window, UICommunicator * uicommunicator, int row, int col) {
  this->uicommunicator = uicommunicator;
  sitename = uicommunicator->getArg1();
  threadid = global->str2Int(uicommunicator->getArg2());
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
        uicommunicator->newCommand("rawdatajump", sitename, global->int2Str(threadid + 1));
      }
      break;
    case KEY_LEFT:
      if (threadid == 0) {
        uicommunicator->newCommand("return");
      }
      else {
        uicommunicator->newCommand("rawdatajump", sitename, global->int2Str(threadid - 1));
      }
      break;
    case 10:
      uicommunicator->newCommand("return");
      break;
  }
}

std::string RawDataScreen::getLegendText() {
  return "[Left] Previous screen - [Right] Next screen - [Enter] Return";
}
