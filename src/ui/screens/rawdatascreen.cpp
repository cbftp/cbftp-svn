#include "rawdatascreen.h"

RawDataScreen::RawDataScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  sitename = uicommunicator->getArg1();
  threadid = global->str2Int(uicommunicator->getArg2());
  uicommunicator->expectBackendPush();
  sitethread = global->getSiteThreadManager()->getSiteThread(sitename);
  threads = sitethread->getConns()->size();
  this->rawbuf = sitethread->getConn(threadid)->getRawBuffer();
  rawbuf->uiWatching(true);
  readfromcopy = false;
  rawcommandmode = false;
  rawcommandswitch = false;
  copyreadpos = 0;
  init(window, row, col);
}

void RawDataScreen::redraw() {
  werase(window);
  if (rawcommandmode) {
    std::string oldtext = rawcommandfield.getData();
    rawcommandfield = MenuSelectOptionTextField("rawcommand", row-1, 10, "", oldtext, col-10, 65536, false);
  }
  update();
}

void RawDataScreen::update() {
  if (rawcommandswitch) {
    if (rawcommandmode) {
      rawcommandmode = false;
      curs_set(0);
    }
    else {
      rawcommandmode = true;
      curs_set(1);
    }
    rawcommandswitch = false;
    redraw();
    return;
  }
  werase(window);
  unsigned int rownum = row;
  if (rawcommandmode) {
    rownum = row - 1;
  }
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
  if (rawcommandmode) {
    std::string pretag = "[Raw command]: ";
    TermInt::printStr(window, rownum, 0, pretag + rawcommandfield.getContentText());

    TermInt::moveCursor(window, rownum, pretag.length() + rawcommandfield.cursorPosition());
  }
}

void RawDataScreen::keyPressed(unsigned int ch) {
  if (rawcommandmode) {
    if ((ch >= 32 && ch <= 126) || ch == 10 || ch == 27 || ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_BACKSPACE || ch == 8) {
      if ((ch >= 32 && ch <= 126) || ch == KEY_BACKSPACE || ch == 8) {
        rawcommandfield.inputChar(ch);
      }
      else if (ch == 10) {
        std::string command = rawcommandfield.getData();
        if (command == "") {
          rawcommandswitch = true;
          uicommunicator->newCommand("updatesetlegend");
          return;
        }
        else {
          readfromcopy = false;
          sitethread->issueRawCommand(threadid, command);
          rawcommandfield.clear();
        }
      }
      else if (ch == 27) {
        if (rawcommandfield.getData() != "") {
          rawcommandfield.clear();
        }
        else {
          rawcommandswitch = true;
          uicommunicator->newCommand("updatesetlegend");
          return;
        }
      }
      else if (ch == KEY_LEFT) {
      }
      else if (ch == KEY_RIGHT) {

      }
      uicommunicator->newCommand("update");
      return;
    }
  }
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
    case 'c':
      sitethread->connectThread(threadid);
      break;
    case 'd':
      sitethread->disconnectThread(threadid);
      break;
    case 'w':
      rawcommandswitch = true;
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 27: // esc
      rawbuf->uiWatching(false);
      uicommunicator->newCommand("return");
      break;

  }
}

std::string RawDataScreen::getLegendText() {
  if (rawcommandmode) {
    return "[Enter] Send command - [Pgup] Scroll up - [Pgdn] Scroll down - [ESC] clear / exit - [Any] Input to text";
  }
  return "[Left] Previous screen - [Right] Next screen - [Enter] Return - [Pgup] Scroll up - [Pgdn] Scroll down - [c]onnect - [d]isconnect - ra[w] command";
}
