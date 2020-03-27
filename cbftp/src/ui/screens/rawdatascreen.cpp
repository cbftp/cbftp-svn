#include "rawdatascreen.h"

#include "../../ftpconn.h"
#include "../../rawbuffer.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../globalcontext.h"
#include "../../encoding.h"

#include "../ui.h"
#include "../termint.h"

namespace {

enum KeyAction {
  KEYACTION_CLEAR,
  KEYACTION_CONNECT,
  KEYACTION_DISCONNECT,
  KEYACTION_RAW_COMMAND
};

enum KeyScopes {
  KEYSCOPE_INPUT,
  KEYSCOPE_NOT_INPUT
};

}

RawDataScreen::RawDataScreen(Ui* ui) : UIWindow(ui, "RawDataScreen") {
  keybinds.addScope(KEYSCOPE_INPUT, "When entering command");
  keybinds.addScope(KEYSCOPE_NOT_INPUT, "When not entering command");
  keybinds.addBind(KEY_PPAGE, KEYACTION_PREVIOUS_PAGE, "Scroll up");
  keybinds.addBind(KEY_NPAGE, KEYACTION_NEXT_PAGE, "Scroll down");
  keybinds.addBind(TERMINT_CTRL_L, KEYACTION_CLEAR, "Clear log");
  keybinds.addBind(10, KEYACTION_ENTER, "Send command", KEYSCOPE_INPUT);
  keybinds.addBind(27, KEYACTION_BACK_CANCEL, "Clear/Cancel", KEYSCOPE_INPUT);
  keybinds.addBind(KEY_UP, KEYACTION_UP, "History up", KEYSCOPE_INPUT);
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "History down", KEYSCOPE_INPUT);
  keybinds.addBind('w', KEYACTION_RAW_COMMAND, "Raw command", KEYSCOPE_NOT_INPUT);
  keybinds.addBind(KEY_LEFT, KEYACTION_LEFT, "Previous screen", KEYSCOPE_NOT_INPUT);
  keybinds.addBind(KEY_RIGHT, KEYACTION_RIGHT, "Next screen", KEYSCOPE_NOT_INPUT);
  keybinds.addBind(10, KEYACTION_BACK_CANCEL, "Return", KEYSCOPE_NOT_INPUT);
  keybinds.addBind('c', KEYACTION_CONNECT, "Connect", KEYSCOPE_NOT_INPUT);
  keybinds.addBind('d', KEYACTION_DISCONNECT, "Disconnect", KEYSCOPE_NOT_INPUT);
  allowimplicitgokeybinds = false;
}

void RawDataScreen::initialize(unsigned int row, unsigned int col, std::string sitename, int connid) {
  this->sitename = sitename;
  this->connid = connid;
  expectbackendpush = true;
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitename);
  threads = sitelogic->getConns()->size();
  this->rawbuf = sitelogic->getConn(connid)->getRawBuffer();
  rawbuf->setUiWatching(true);
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
  printRawBufferLines(ui, rawbuf, rownum, col, 0, readfromcopy, copysize, copyreadpos);
  if (rawcommandmode) {
    std::string pretag = "[Raw command]: ";
    ui->printStr(rownum, 0, pretag + rawcommandfield.getContentText());
    ui->moveCursor(rownum, pretag.length() + rawcommandfield.cursorPosition());
  }
}

void RawDataScreen::printRawBufferLines(Ui * ui, RawBuffer * rawbuf, unsigned int rownum, unsigned int col, unsigned int coloffset) {
  printRawBufferLines(ui, rawbuf, rownum, col, coloffset, false, 0, 0);
}

void RawDataScreen::printRawBufferLines(Ui * ui, RawBuffer * rawbuf, unsigned int rownum, unsigned int col, unsigned int coloffset, bool readfromcopy, unsigned int copysize, unsigned int copyreadpos) {
  bool cutfirst5 = false;
  bool skiptag = false;
  bool skiptagchecked = false;
  unsigned int numlinestoprint = !readfromcopy
                                 ? (rawbuf->getSize() < rownum ? rawbuf->getSize() : rownum)
                                 : (copysize < rownum ? copysize : rownum);
  for (unsigned int i = 0; i < numlinestoprint; i++) {
    const std::pair<std::string, std::string> & line = !readfromcopy
                            ? rawbuf->getLine(numlinestoprint - i - 1)
                            : rawbuf->getLineCopy(numlinestoprint - i - 1 + copyreadpos);
    if (!skiptagchecked) {
      if (col <= 80 + line.first.length()) {
        skiptag = true;
      }
      skiptagchecked = true;
    }
    if (!cutfirst5 && line.second.length() > col && skipCodePrint(line.second)) {
      cutfirst5 = true;
    }
    if (skiptagchecked && cutfirst5) {
      break;
    }
  }
  for (unsigned int i = 0; i < numlinestoprint; i++) {
    const std::pair<std::string, std::string> & line = !readfromcopy
                            ? rawbuf->getLine(numlinestoprint - i - 1)
                            : rawbuf->getLineCopy(numlinestoprint - i - 1 + copyreadpos);
    unsigned int startprintsecond = 0;
    if (!skiptag) {
      unsigned int length = line.first.length();
      ui->printStr(i, coloffset, line.first, false, col - coloffset);
      startprintsecond = length + 1;
    }
    unsigned int start = 0;
    if (cutfirst5 && skipCodePrint(line.second)) {
      start = 5;
    }
    ui->printStr(i, startprintsecond + coloffset, encoding::cp437toUnicode(line.second.substr(start)), false, col - coloffset - startprintsecond);
  }
}

bool RawDataScreen::skipCodePrint(const std::string & line) {
  return line.length() >= 5 && (line.substr(0, 5) == "230- " || line.substr(0, 5) == "200- ");
}

bool RawDataScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch, rawcommandmode ? KEYSCOPE_INPUT : KEYSCOPE_NOT_INPUT);
  unsigned int rownum = row;
  if (action == KEYACTION_CLEAR) {
    rawbuf->clear();
    readfromcopy = false;
    ui->redraw();
    return true;
  }
  if (rawcommandmode) {
    rownum = row - 1;
    if ((ch >= 32 && ch <= 126) || ch == KEY_BACKSPACE || ch == 8 || ch == 127 ||
        ch == KEY_RIGHT || ch == KEY_LEFT || ch == KEY_DC || ch == KEY_HOME ||
        ch == KEY_END) {
      rawcommandfield.inputChar(ch);
      ui->update();
      return true;
    }
    else if (action == KEYACTION_ENTER) {
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
    else if (action == KEYACTION_BACK_CANCEL) {
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
    else if (action == KEYACTION_UP) {
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
    else if (action == KEYACTION_DOWN) {
      if (history.forward()) {
        rawcommandfield.setText(history.get());
        ui->update();
      }
      return true;
    }
  }
  switch(action) {
    case KEYACTION_RIGHT:
      if (connid + 1 < threads) {
        rawbuf->setUiWatching(false);
        ui->goRawDataJump(sitename, connid + 1);
      }
      return true;
    case KEYACTION_LEFT:
      if (connid == 0) {
        rawbuf->setUiWatching(false);
        ui->returnToLast();
      }
      else {
        rawbuf->setUiWatching(false);
        ui->goRawDataJump(sitename, connid - 1);
      }
      return true;
    case KEYACTION_PREVIOUS_PAGE:
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
    case KEYACTION_NEXT_PAGE:
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
    case KEYACTION_CONNECT:
      sitelogic->connectConn(connid);
      return true;
    case KEYACTION_DISCONNECT:
      sitelogic->disconnectConn(connid);
      return true;
    case KEYACTION_RAW_COMMAND:
      rawcommandswitch = true;
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_BACK_CANCEL: // esc
      rawbuf->setUiWatching(false);
      ui->returnToLast();
      return true;
    case KEYACTION_KEYBINDS:
      ui->goKeyBinds(&keybinds);
      return true;
  }
  return false;
}

std::string RawDataScreen::getLegendText() const {
  return keybinds.getLegendSummary(rawcommandmode ? KEYSCOPE_INPUT : KEYSCOPE_NOT_INPUT);
}

std::string RawDataScreen::getInfoLabel() const {
  return "RAW DATA: " + sitename + " #" + std::to_string(connid);
}
