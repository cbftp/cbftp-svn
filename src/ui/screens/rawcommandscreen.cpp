#include "rawcommandscreen.h"

RawCommandScreen::RawCommandScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  sitename = uicommunicator->getArg1();
  uicommunicator->expectBackendPush();
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitename);
  this->rawbuf = sitelogic->getRawCommandBuffer();
  rawbuf->uiWatching(true);
  readfromcopy = false;
  copyreadpos = 0;
  init(window, row, col);
}

void RawCommandScreen::redraw() {
  werase(window);
  std::string oldtext = rawcommandfield.getData();
  rawcommandfield = MenuSelectOptionTextField("rawcommand", row-1, 10, "", oldtext, col-10, 65536, false);
  update();
}

void RawCommandScreen::update() {
  werase(window);
  curs_set(1);
  unsigned int rownum = row - 1;
  if (!readfromcopy) {
    unsigned int numlinestoprint = rawbuf->getSize() < rownum ? rawbuf->getSize() : rownum;
    for (unsigned int i = 0; i < numlinestoprint; i++) {
      TermInt::printStr(window, i, 0, rawbuf->getLine(numlinestoprint - i - 1));
    }
  }
  else {
    unsigned int numlinestoprint = copysize < rownum ? copysize : rownum;
    for (unsigned int i = 0; i < numlinestoprint; i++) {
      TermInt::printStr(window, i, 0, rawbuf->getLineCopy(numlinestoprint - i - 1 + copyreadpos));
    }
  }
  std::string pretag = "[Raw command]: ";
  TermInt::printStr(window, rownum, 0, pretag + rawcommandfield.getContentText());
  TermInt::moveCursor(window, rownum, pretag.length() + rawcommandfield.cursorPosition());
}

void RawCommandScreen::keyPressed(unsigned int ch) {
  unsigned int rownum = row - 1;
  if ((ch >= 32 && ch <= 126) || ch == KEY_BACKSPACE || ch == 8) {
    rawcommandfield.inputChar(ch);
    uicommunicator->newCommand("update");
  }
  else {
    std::string command;
    switch(ch) {
      case 10:
          command = rawcommandfield.getData();
          if (command != "") {
            readfromcopy = false;
            sitelogic->requestRawCommand(command);
            rawcommandfield.clear();
            uicommunicator->newCommand("update");
          }
          break;
      case 27: // esc
        if (rawcommandfield.getData() != "") {
          rawcommandfield.clear();
          uicommunicator->newCommand("update");
        }
        else {
          rawbuf->uiWatching(false);
          uicommunicator->newCommand("return");
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
          copyreadpos = copyreadpos + rownum / 2;
          if (rownum >= copysize) {
            copyreadpos = 0;
          }
          else if (copyreadpos + rownum > copysize) {
            copyreadpos = copysize - rownum;
          }
        }
        uicommunicator->newCommand("update");
        break;
      case KEY_NPAGE:
        if (readfromcopy) {
          if (copyreadpos == 0) {
            readfromcopy = false;
          }
          else if (copyreadpos < rownum / 2) {
            copyreadpos = 0;
          }
          else {
            copyreadpos = copyreadpos - rownum / 2;
          }
        }
        uicommunicator->newCommand("update");
        break;
    }
  }
}

std::string RawCommandScreen::getLegendText() {
  return "[Enter] Send command - [ESC] clear / exit - [Any] Input to text - [Pgup] Scroll up - [Pgdn] Scroll down";
}

std::string RawCommandScreen::getInfoLabel() {
  return "RAW COMMAND INPUT: " + sitename;
}
