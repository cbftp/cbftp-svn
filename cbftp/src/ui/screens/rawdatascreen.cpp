#include "rawdatascreen.h"

#include "../../ftpconn.h"
#include "../../rawbuffer.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../globalcontext.h"
#include "../../util.h"
#include "../../encoding.h"

#include "../ui.h"

extern GlobalContext * global;

RawDataScreen::RawDataScreen(Ui * ui) {
  this->ui = ui;
}

void RawDataScreen::initialize(unsigned int row, unsigned int col, std::string sitename, int connid) {
  this->sitename = sitename;
  this->connid = connid;
  expectbackendpush = true;
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitename);
  threads = sitelogic->getConns()->size();
  this->rawbuf = sitelogic->getConn(connid)->getRawBuffer();
  rawbuf->uiWatching(true);
  readfromcopy = false;
  rawcommandmode = false;
  rawcommandswitch = false;
  copyreadpos = 0;
  init(row, col);
}

void RawDataScreen::redraw() {
  ui->erase();
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
      ui->hideCursor();
    }
    else {
      rawcommandmode = true;
      ui->showCursor();
    }
    rawcommandswitch = false;
    redraw();
    return;
  }
  ui->erase();
  unsigned int rownum = row;
  if (rawcommandmode) {
    rownum = row - 1;
  }
  printRawBufferLines(ui, rawbuf, rownum, col, readfromcopy, copysize, copyreadpos);
  if (rawcommandmode) {
    std::string pretag = "[Raw command]: ";
    ui->printStr(rownum, 0, pretag + rawcommandfield.getContentText());
    ui->moveCursor(rownum, pretag.length() + rawcommandfield.cursorPosition());
  }
}

void RawDataScreen::printRawBufferLines(Ui * ui, RawBuffer * rawbuf, unsigned int rownum, unsigned int col, bool readfromcopy, unsigned int copysize, unsigned int copyreadpos) {
  std::list<std::string> lines;
  bool cutfirst5 = false;
  if (!readfromcopy) {
    unsigned int numlinestoprint = rawbuf->getSize() < rownum ? rawbuf->getSize() : rownum;
    for (unsigned int i = 0; i < numlinestoprint; i++) {
      std::string line = makeLine(col, rawbuf->getLine(numlinestoprint - i - 1));
      if (!cutfirst5 && line.length() > col && skipCodePrint(line)) {
        cutfirst5 = true;
      }
      lines.push_back(line);
    }
  }
  else {
    unsigned int numlinestoprint = copysize < rownum ? copysize : rownum;
    for (unsigned int i = 0; i < numlinestoprint; i++) {
      std::string line = makeLine(col, rawbuf->getLineCopy(numlinestoprint - i - 1 + copyreadpos));
      if (!cutfirst5 && line.length() > col && skipCodePrint(line)) {
        cutfirst5 = true;
      }
      lines.push_back(line);
    }
  }
  unsigned int i = 0;
  for (std::list<std::string>::const_iterator it = lines.begin(); it != lines.end(); it++, i++) {
    unsigned int start = 0;
    if (cutfirst5 && skipCodePrint(*it)) {
      start = 5;
    }
    for (unsigned int j = start; j < it->length(); j++) {
      ui->printChar(i, j - start, encoding::cp437toUnicode((*it)[j]));
    }
  }
}

std::string RawDataScreen::makeLine(unsigned int col, std::pair<std::string, std::string> entry) {
  return (col > 80 + entry.first.length() ? entry.first + " " : "") + entry.second;
}

bool RawDataScreen::skipCodePrint(const std::string & line) {
  return line.length() >= 5 && (line.substr(0, 5) == "230- " || line.substr(0, 5) == "200- ");
}

bool RawDataScreen::keyPressed(unsigned int ch) {
  unsigned int rownum = row;
  if (rawcommandmode) {
    rownum = row - 1;
    if ((ch >= 32 && ch <= 126) || ch == KEY_BACKSPACE || ch == 8 || ch == 127 ||
        ch == KEY_RIGHT || ch == KEY_LEFT || ch == KEY_DC || ch == KEY_HOME ||
        ch == KEY_END) {
      rawcommandfield.inputChar(ch);
      ui->update();
      return true;
    }
    else if (ch == 10) {
      std::string command = rawcommandfield.getData();
      if (command == "") {
        rawcommandswitch = true;
        ui->update();
        ui->setLegend();
        return true;
      }
      else {
        readfromcopy = false;
        history.push(command);
        sitelogic->issueRawCommand(connid, command);
        rawcommandfield.clear();
      }
      ui->update();
      return true;
    }
    else if (ch == 27) {
      if (rawcommandfield.getData() != "") {
        rawcommandfield.clear();
      }
      else {
        rawcommandswitch = true;
        ui->update();
        ui->setLegend();
        return true;
      }
      ui->update();
      return true;
    }
    else if (ch == KEY_UP) {
      if (history.canBack()) {
        if (history.current()) {
          history.setCurrent(rawcommandfield.getData());
        }
        history.back();
        rawcommandfield.setText(history.get());
        ui->update();
      }
      return true;
    }
    else if (ch == KEY_DOWN) {
      if (history.forward()) {
        rawcommandfield.setText(history.get());
        ui->update();
      }
      return true;
    }
  }
  switch(ch) {
    case KEY_RIGHT:
      if (connid + 1 < threads) {
        rawbuf->uiWatching(false);
        ui->goRawDataJump(sitename, connid + 1);
      }
      return true;
    case KEY_LEFT:
      if (connid == 0) {
        rawbuf->uiWatching(false);
        ui->returnToLast();
      }
      else {
        rawbuf->uiWatching(false);
        ui->goRawDataJump(sitename, connid - 1);
      }
      return true;
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
      ui->update();
      return true;
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
      ui->update();
      return true;
    case 'c':
      sitelogic->connectConn(connid);
      return true;
    case 'd':
      sitelogic->disconnectConn(connid);
      return true;
    case 'w':
      rawcommandswitch = true;
      ui->update();
      ui->setLegend();
      return true;
    case 27: // esc
      rawbuf->uiWatching(false);
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string RawDataScreen::getLegendText() const {
  if (rawcommandmode) {
    return "[Enter] Send command - [Pgup] Scroll up - [Pgdn] Scroll down - [ESC] clear / exit - [Any] Input to text";
  }
  return "[Left] Previous screen - [Right] Next screen - [Enter] Return - [Pgup] Scroll up - [Pgdn] Scroll down - [c]onnect - [d]isconnect - ra[w] command";
}

std::string RawDataScreen::getInfoLabel() const {
  return "RAW DATA: " + sitename + " #" + util::int2Str(connid);
}
