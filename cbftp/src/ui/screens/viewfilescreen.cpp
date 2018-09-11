#include "viewfilescreen.h"

#include "transfersscreen.h"

#include "../ui.h"
#include "../menuselectoption.h"
#include "../resizableelement.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../termint.h"
#include "../misc.h"

#include "../../transferstatus.h"
#include "../../globalcontext.h"
#include "../../sitelogicmanager.h"
#include "../../sitelogic.h"
#include "../../transfermanager.h"
#include "../../localstorage.h"
#include "../../localfilelist.h"
#include "../../filelist.h"
#include "../../file.h"
#include "../../externalfileviewing.h"
#include "../../util.h"
#include "../../core/types.h"

class CommandOwner;

namespace ViewFileState {

enum {
  NO_SLOTS_AVAILABLE,
  TOO_LARGE_FOR_INTERNAL,
  NO_DISPLAY,
  CONNECTING,
  DOWNLOADING,
  LOADING_VIEWER,
  VIEWING_EXTERNAL,
  VIEWING_INTERNAL
};

}

ViewFileScreen::ViewFileScreen(Ui * ui) {
  this->ui = ui;
}

ViewFileScreen::~ViewFileScreen() {

}

void ViewFileScreen::initialize() {
  rawcontents.clear();
  x = 0;
  y = 0;
  ymax = 0;
  xmax = 0;
  externallyviewable = false;
  legendupdated = false;
  pid = 0;
  autoupdate = true;
  ts.reset();
}

void ViewFileScreen::initialize(unsigned int row, unsigned int col, const std::string & site, const std::string & file, FileList * filelist) {
  initialize();
  deleteafter = true;
  this->site = site;
  this->file = file;
  this->filelist = filelist;
  sitelogic = global->getSiteLogicManager()->getSiteLogic(site);
  size = filelist->getFile(file)->getSize();
  state = ViewFileState::CONNECTING;
  if (global->getExternalFileViewing()->isViewable(file)) {
    if (!global->getExternalFileViewing()->hasDisplay()) {
      state = ViewFileState::NO_DISPLAY;
    }
    externallyviewable = true;
  }
  else {
    if (size > MAXOPENSIZE) {
      state = ViewFileState::TOO_LARGE_FOR_INTERNAL;
    }
  }
  if (state == ViewFileState::CONNECTING) {
    requestid = sitelogic->requestOneIdle();
  }
  if (state == ViewFileState::DOWNLOADING) {

    if (!ts) {
      state = ViewFileState::NO_SLOTS_AVAILABLE;
    }
    else {

    }
  }
  init(row, col);
}

void ViewFileScreen::initialize(unsigned int row, unsigned int col, const Path & dir, const std::string & file) {
  initialize();
  deleteafter = false;
  path = dir / file;
  this->file = file;
  size = global->getLocalStorage()->getFileSize(path);
  state = ViewFileState::LOADING_VIEWER;
  if (global->getExternalFileViewing()->isViewable(file)) {
    if (!global->getExternalFileViewing()->hasDisplay()) {
      state = ViewFileState::NO_DISPLAY;
    }
    externallyviewable = true;
  }
  else {
    if (size > MAXOPENSIZE) {
      state = ViewFileState::TOO_LARGE_FOR_INTERNAL;
    }
  }
  init(row, col);
}

void ViewFileScreen::redraw() {
  ui->erase();
  switch (state) {
    case ViewFileState::NO_SLOTS_AVAILABLE:
      ui->printStr(1, 1, "No download slots available at " + site + ".");
      break;
    case ViewFileState::TOO_LARGE_FOR_INTERNAL:
      ui->printStr(1, 1, file + " is too large to download and open in the internal viewer.");
      ui->printStr(2, 1, "The maximum file size for internal viewing is set to " + util::int2Str(MAXOPENSIZE) + " bytes.");
      break;
    case ViewFileState::NO_DISPLAY:
      ui->printStr(1, 1, file + " cannot be opened in an external viewer.");
      ui->printStr(2, 1, "The DISPLAY environment variable is not set.");
      break;
    case ViewFileState::CONNECTING:
      if (sitelogic->requestReady(requestid)) {
        sitelogic->finishRequest(requestid);
        const Path & temppath = global->getLocalStorage()->getTempPath();
        std::shared_ptr<LocalFileList> localfl = global->getLocalStorage()->getLocalFileList(temppath);
        ts = global->getTransferManager()->suggestDownload(file, sitelogic, filelist, localfl, NULL);
        if (!!ts) {
          state = ViewFileState::DOWNLOADING;
          ts->setAwaited(true);
          path = temppath / file;
          expectbackendpush = true;
        }
        else {
          state = ViewFileState::NO_SLOTS_AVAILABLE;
          redraw();
        }
      }
      else {
        ui->printStr(1, 1, "Awaiting slot...");
      }
      break;
    case ViewFileState::DOWNLOADING:
      switch(ts->getState()) {
        case TRANSFERSTATUS_STATE_IN_PROGRESS:
          ui->printStr(1, 1, "Downloading from " + site + "...");
          printTransferInfo();
          break;
        case TRANSFERSTATUS_STATE_FAILED:
          ui->printStr(1, 1, "Download of " + file + " from " + site + " failed.");
          autoupdate = false;
          break;
        case TRANSFERSTATUS_STATE_SUCCESSFUL:
          loadViewer();
          break;
      }
      break;
    case ViewFileState::LOADING_VIEWER:
      loadViewer();
      break;
    case ViewFileState::VIEWING_EXTERNAL:
      viewExternal();
      break;
    case ViewFileState::VIEWING_INTERNAL:
      viewInternal();
      break;
  }
}

void ViewFileScreen::update() {
  if (pid) {
    if (!global->getExternalFileViewing()->stillViewing(pid)) {
      ui->returnToLast();
    }
    else if (!legendupdated) {
      legendupdated = true;
      ui->update();
      ui->setLegend();
    }
  }
  else {
    redraw();
    if ((state == ViewFileState::VIEWING_INTERNAL || state == ViewFileState::VIEWING_EXTERNAL)
        && !legendupdated)
    {
      legendupdated = true;
      ui->update();
      ui->setLegend();
    }
  }
}

bool ViewFileScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case ' ':
    case 'c':
    case 10:
      ui->returnToLast();
      return true;
    case KEY_DOWN:
      if (goDown()) {
        goDown();
        ui->setInfo();
        ui->redraw();
      }
      return true;
    case KEY_UP:
      if (goUp()) {
        goUp();
        ui->setInfo();
        ui->redraw();
      }
      return true;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < row / 2; i++) {
        goDown();
      }
      ui->setInfo();
      ui->redraw();
      return true;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < row / 2; i++) {
        goUp();
      }
      ui->setInfo();
      ui->redraw();
      return true;
    case 'k':
      if (pid) {
        global->getExternalFileViewing()->killProcess(pid);
      }
      return true;
    case 'e':
      if (state == ViewFileState::VIEWING_INTERNAL) {
        if (encoding == encoding::ENCODING_CP437) {
          encoding = encoding::ENCODING_CP437_DOUBLE;
        }
        else if (encoding == encoding::ENCODING_CP437_DOUBLE) {
          encoding = encoding::ENCODING_ISO88591;
        }
        else if (encoding == encoding::ENCODING_ISO88591) {
          encoding = encoding::ENCODING_UTF8;
        }
        else {
          encoding = encoding::ENCODING_CP437;
        }
        translate();
        ui->redraw();
        ui->setInfo();
      }
      return true;
  }
  return false;
}

void ViewFileScreen::loadViewer() {
  if (externallyviewable) {
    if (!pid) {
      if (deleteafter) {
        pid = global->getExternalFileViewing()->viewThenDelete(path);
      }
      else {
        pid = global->getExternalFileViewing()->view(path);
      }
    }
    state = ViewFileState::VIEWING_EXTERNAL;
    viewExternal();
  }
  else {
    BinaryData tmpdata = global->getLocalStorage()->getFileContent(path);
    if (deleteafter) {
      global->getLocalStorage()->deleteFile(path);
    }
    std::string extension = File::getExtension(file);
    encoding = encoding::guessEncoding(tmpdata);
    unsigned int tmpdatalen = tmpdata.size();
    if (tmpdatalen > 0) {
      std::string current;
      for (unsigned int i = 0; i < tmpdatalen; i++) {
        if (tmpdata[i] == '\n') {
          rawcontents.push_back(current);
          current.clear();
        }
        else {
          current += tmpdata[i];
        }
      }
      if (current.length() > 0) {
        rawcontents.push_back(current);
      }
      translate();
    }
    autoupdate = false;
    state = ViewFileState::VIEWING_INTERNAL;
    viewInternal();
  }
}

void ViewFileScreen::viewExternal() {
  ui->printStr(1, 1, "Opening " + file + " with: " + global->getExternalFileViewing()->getViewApplication(file));
  ui->printStr(3, 1, "Press 'k' to kill this external viewer instance.");
  ui->printStr(4, 1, "You can always press 'K' to kill ALL external viewers.");
}

void ViewFileScreen::viewInternal() {
  if (ymax <= row) {
    y = 0;
  }
  while (ymax > row && y + row > ymax) {
    --y;
  }
  ymax = rawcontents.size();
  for (unsigned int i = 0; i < ymax; i++) {
    if (translatedcontents[i].length() > xmax) {
      xmax = translatedcontents[i].length();
    }
  }
  for (unsigned int i = 0; i < row && i < ymax; i++) {
    std::basic_string<unsigned int> & line = translatedcontents[y + i];
    for (unsigned int j = 0; j < line.length() && j < col - 2; j++) {
      ui->printChar(i, j + 1, line[j]);
    }
  }

  printSlider(ui, row, col - 1, ymax, y);
}

std::string ViewFileScreen::getLegendText() const {
  if (state == ViewFileState::VIEWING_EXTERNAL) {
    return "[Esc/Enter/c] Return - [k]ill external viewer - [K]ill ALL external viewers";
  }
  if (state == ViewFileState::VIEWING_INTERNAL) {
    return "[Arrowkeys] Navigate - [Esc/Enter/c] Return - switch [e]ncoding";
  }
  return "[Esc/Enter/c] Return";
}

std::string ViewFileScreen::getInfoLabel() const {
  return "VIEW FILE: " + file;
}

std::string ViewFileScreen::getInfoText() const {
  if (state == ViewFileState::VIEWING_INTERNAL) {
    std::string enc;
    switch (encoding) {
      case encoding::ENCODING_CP437:
        enc = "CP437";
        break;
      case encoding::ENCODING_CP437_DOUBLE:
        enc = "Double CP437";
        break;
      case encoding::ENCODING_ISO88591:
        enc = "ISO-8859-1";
        break;
      case encoding::ENCODING_UTF8:
        enc = "UTF-8";
        break;
    }
    unsigned int end = ymax < y + row ? ymax : y + row;
    return "Line " + util::int2Str(y) + "-" +
        util::int2Str(end) + "/" + util::int2Str(ymax) + "  Encoding: " + enc;
  }
  else {
    return "";
  }
}

void ViewFileScreen::translate() {
  translatedcontents.clear();
  for (unsigned int i = 0; i < rawcontents.size(); i++) {
    std::basic_string<unsigned int> current;
    if (encoding == encoding::ENCODING_CP437_DOUBLE) {
      current = encoding::doublecp437toUnicode(rawcontents[i]);
    }
    else if (encoding == encoding::ENCODING_CP437) {
      current = encoding::cp437toUnicode(rawcontents[i]);
    }
    else if (encoding == encoding::ENCODING_ISO88591) {
      current = encoding::toUnicode(rawcontents[i]);
    }
    else {
      current = encoding::utf8toUnicode(rawcontents[i]);
    }
    translatedcontents.push_back(current);
  }
}

bool ViewFileScreen::goDown() {
  if (y + row < ymax) {
    y++;
    return true;
  }
  return false;
}

bool ViewFileScreen::goUp() {
  if (y > 0) {
    y--;
    return true;
  }
  return false;
}

void ViewFileScreen::printTransferInfo() {
  TransferDetails td = TransfersScreen::formatTransferDetails(ts);
  unsigned int y = 3;
  MenuSelectOption table;
  std::shared_ptr<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  std::shared_ptr<MenuSelectOptionTextButton> msotb;
  msotb = table.addTextButtonNoContent(y, 1, "transferred", "TRANSFERRED");
  msal->addElement(msotb, 4, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 3, "filename", "FILENAME");
  msal->addElement(msotb, 2, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 6, "remaining", "LEFT");
  msal->addElement(msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 7, "speed", "SPEED");
  msal->addElement(msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 8, "progress", "DONE");
  msal->addElement(msotb, 7, RESIZE_REMOVE);
  y++;
  msal = table.addAdjustableLine();
  msotb = table.addTextButtonNoContent(y, 1, "transferred", td.transferred);
  msal->addElement(msotb, 4, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 10, "filename", ts->getFile());
  msal->addElement(msotb, 2, RESIZE_WITHLAST3);
  msotb = table.addTextButtonNoContent(y, 60, "remaining", td.timeremaining);
  msal->addElement(msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 40, "speed", td.speed);
  msal->addElement(msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 50, "progress", td.progress);
  msal->addElement(msotb, 7, RESIZE_REMOVE);
  table.adjustLines(col - 3);
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
    if (re->isVisible()) {
      if (re->getIdentifier() == "transferred") {
        std::string labeltext = re->getLabelText();
        bool highlight = table.getLineIndex(table.getAdjustableLine(re)) == 1;
        int charswithhighlight = highlight ? labeltext.length() * ts->getProgress() / 100 : 0;
        ui->printStr(re->getRow(), re->getCol(), labeltext.substr(0, charswithhighlight), true);
        ui->printStr(re->getRow(), re->getCol() + charswithhighlight, labeltext.substr(charswithhighlight));
      }
      else {
        ui->printStr(re->getRow(), re->getCol(), re->getLabelText());
      }
    }
  }
}
