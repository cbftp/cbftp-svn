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
  this->sitename = sitename;
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
  rawbuf->uiWatching(true);
  readfromcopy = false;
  copyreadpos = 0;
  init(row, col);
}

void RawCommandScreen::initialize(unsigned int row, unsigned int col, const std::string & sitename) {
  Pointer<SiteLogic> sl = global->getSiteLogicManager()->getSiteLogic(sitename);
  initialize(row, col, sitename, sl->getSite()->getBasePath(), "");
}

void RawCommandScreen::redraw() {
  ui->erase();
  std::string oldtext = rawcommandfield.getData();
  rawcommandfield = MenuSelectOptionTextField("rawcommand", row-1, 10, "", oldtext, col-10, 65536, false);
  update();
}

void RawCommandScreen::update() {
  ui->erase();
  ui->showCursor();
  unsigned int rownum = row - 1;
  RawDataScreen::printRawBufferLines(ui, rawbuf, rownum, col, 0, readfromcopy, copysize, copyreadpos);
  std::string pretag = "[Raw command]: ";
  ui->printStr(rownum, 0, pretag + rawcommandfield.getContentText());
  ui->moveCursor(rownum, pretag.length() + rawcommandfield.cursorPosition());
}

bool RawCommandScreen::keyPressed(unsigned int ch) {
  unsigned int rownum = row - 1;
  if ((ch >= 32 && ch <= 126) || ch == KEY_BACKSPACE || ch == 8 || ch == 127 ||
      ch == KEY_LEFT || ch == KEY_RIGHT || ch == KEY_HOME || ch == KEY_END ||
      ch == KEY_DC) {
    rawcommandfield.inputChar(ch);
    ui->update();
    return true;
  }
  else {
    std::string command;
    switch(ch) {
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
      case 27: // esc
        if (rawcommandfield.getData() != "") {
          rawcommandfield.clear();
          ui->update();
        }
        else {
          rawbuf->uiWatching(false);
          ui->returnToLast();
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
      case KEY_IC:
        for (unsigned int i = 0; i < selection.length(); i++) {
          rawcommandfield.inputChar(selection[i]);
        }
        ui->update();
        return true;
    }
  }
  return false;
}

std::string RawCommandScreen::getLegendText() const {
  std::string legendtext = "[Enter] Send command - [ESC] clear / exit - [Any] Input to text - [Pgup] Scroll up - [Pgdn] Scroll down";
  if (hasselection) {
    legendtext += " - [Insert] selection";
  }
  return legendtext;
}

std::string RawCommandScreen::getInfoLabel() const {
  return "RAW COMMAND INPUT: " + sitename;
}

std::string RawCommandScreen::getInfoText() const {
  return path.toString();
}
