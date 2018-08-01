#include "rawcommandscreen.h"

#include "../../ftpconn.h"
#include "../../rawbuffer.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../globalcontext.h"
#include "../../encoding.h"
#include "../../site.h"

#include "../ui.h"

#include "rawdatascreen.h"

RawCommandScreen::RawCommandScreen(Ui * ui) {
  this->ui = ui;
}

void RawCommandScreen::initialize(unsigned int row, unsigned int col, const std::string & sitename, const Path & path, const std::string & selection) {
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitename);
  this->label = "RAW COMMAND INPUT: " + sitename;
  this->selection = selection;
  this->path = path;
  if (this->path == "") {
    this->path = "/";
  }
  hasselection = false;
  if (selection.length()) {
    hasselection = true;
  }
  expectbackendpush = true;

  this->rawbuf = sitelogic->getRawCommandBuffer();
  rawbuf->setUiWatching(true);
  readfromcopy = false;
  copyreadpos = 0;
  allowinput = true;
  init(row, col);
}

void RawCommandScreen::initialize(unsigned int row, unsigned int col, const std::string & sitename) {
  Pointer<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(sitename);
  initialize(row, col, sitename, sl->getSite()->getBasePath(), "");
}

void RawCommandScreen::initialize(unsigned int row, unsigned int col, RawBuffer * rawbuffer, const std::string & label, const std::string & infotext) {
  this->label = label;
  this->path = infotext;
  selection = "";
  hasselection = false;
  expectbackendpush = false;
  this->rawbuf = rawbuffer;
  readfromcopy = false;
  copyreadpos = 0;
  allowinput = false;
  init(row, col);
}

void RawCommandScreen::redraw() {
  ui->erase();
  std::string oldtext = rawcommandfield.getData();
  rawcommandfield = MenuSelectOptionTextField("rawcommand", row-1, 10, "", oldtext, col-10, 65536, false);
  update();
}

void RawCommandScreen::update() {
  ui->erase();
  unsigned int rownum = row;
  if (allowinput) {
    rownum--;
    ui->showCursor();
    std::string pretag = "[Raw command]: ";
    ui->printStr(rownum, 0, pretag + rawcommandfield.getContentText());
    ui->moveCursor(rownum, pretag.length() + rawcommandfield.cursorPosition());
  }
  RawDataScreen::printRawBufferLines(ui, rawbuf, rownum, col, 0, readfromcopy, copysize, copyreadpos);
}

bool RawCommandScreen::keyPressed(unsigned int ch) {
  unsigned int rownum = allowinput ? row - 1 : row;
  std::string command;
  switch(ch) {
    case 27: // esc
      if (rawcommandfield.getData() != "") {
        rawcommandfield.clear();
        ui->update();
      }
      else {
        rawbuf->setUiWatching(false);
        ui->returnToLast();
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

  }
  if (!allowinput) {
    return false;
  }
  if ((ch >= 32 && ch <= 126) || ch == KEY_BACKSPACE || ch == 8 || ch == 127 ||
      ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_HOME || ch == KEY_END ||
      ch == KEY_DC)
  {
    rawcommandfield.inputChar(ch);
    ui->update();
    return true;
  }
  switch (ch) {
    case 10:
      command = rawcommandfield.getData();
      if (command != "") {
        readfromcopy = false;
        history.push(command);
        sitelogic->requestRawCommand(path, command, true);
        rawcommandfield.clear();
        ui->update();
      }
      return true;
    case KEY_UP:
      if (history.canBack()) {
        if (history.current()) {
          history.setCurrent(rawcommandfield.getData());
        }
        history.back();
        rawcommandfield.setText(history.get());
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (history.forward()) {
        rawcommandfield.setText(history.get());
        ui->update();
      }
      return true;
    case KEY_IC:
      if (!allowinput) {
        return false;
      }
      for (unsigned int i = 0; i < selection.length(); i++) {
        rawcommandfield.inputChar(selection[i]);
      }
      ui->update();
      return true;
  }
  return false;
}

std::string RawCommandScreen::getLegendText() const {
  std::string legendtext = "[ESC] exit - [Pgup] Scroll up - [Pgdn] Scroll down";
  if (allowinput) {
    legendtext = "[ESC] clear / exit - [Pgup] Scroll up - [Pgdn] Scroll down - [Any] Input to text - [Enter] Send command";
    if (hasselection) {
      legendtext += " - [Insert] selection";
    }
  }
  return legendtext;
}

std::string RawCommandScreen::getInfoLabel() const {
  return label;
}

std::string RawCommandScreen::getInfoText() const {
  return path.toString();
}
