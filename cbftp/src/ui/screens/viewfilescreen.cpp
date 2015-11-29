#include "viewfilescreen.h"

#include "../ui.h"
#include "../menuselectoption.h"
#include "../resizableelement.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"
#include "../termint.h"

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
#include "../../types.h"


extern GlobalContext * global;

ViewFileScreen::ViewFileScreen(Ui * ui) {
  this->ui = ui;
}

ViewFileScreen::~ViewFileScreen() {

}

void ViewFileScreen::initialize(unsigned int row, unsigned int col, std::string site, std::string file, FileList * filelist) {
  rawcontents.clear();
  viewingcontents = false;
  x = 0;
  y = 0;
  ymax = 0;
  xmax = 0;
  this->site = site;
  this->file = file;
  this->filelist = filelist;
  sitelogic = global->getSiteLogicManager()->getSiteLogic(site);
  size = filelist->getFile(file)->getSize();
  hasnodisplay = false;
  externallyviewable = false;
  download = false;
  legendupdated = false;
  downloadattempted = false;
  pid = 0;
  autoupdate = true;
  ts.reset();
  if (!global->getExternalFileViewing()->hasDisplay()) {
    hasnodisplay = true;
  }
  if (global->getExternalFileViewing()->isViewable(file)) {
    externallyviewable = true;
    if (!hasnodisplay) {
      download = true;
    }
  }
  else {
    if (size <= MAXOPENSIZE) {
      download = true;
    }
  }
  std::string temppath = global->getLocalStorage()->getTempPath();
  Pointer<LocalFileList> localfl = global->getLocalStorage()->getLocalFileList(temppath);
  if (download) {
    downloadattempted = true;
    ts = global->getTransferManager()->suggestDownload(file, sitelogic,
        filelist, localfl);
    if (!ts) {
      download = false;
    }
    else {
      ts->setAwaited(true);
    }
  }
  path = temppath + "/" + file;
  expectbackendpush = true;
  init(row, col);
}

void ViewFileScreen::redraw() {
  ui->erase();
  if (!download) {
    autoupdate = false;
    if (downloadattempted) {
      ui->printStr(1, 1, "No download slots available at " + site + ".");
    }
    else if (!externallyviewable) {
      ui->printStr(1, 1, file + " is too large to download and open in the internal viewer.");
      ui->printStr(2, 1, "The maximum file size for internal viewing is set to " + util::int2Str(MAXOPENSIZE) + " bytes.");
    }
    else if (hasnodisplay) {
      ui->printStr(1, 1, file + " cannot be opened in an external viewer.");
      ui->printStr(2, 1, "The DISPLAY environment variable is not set.");
    }

    return;
  }
  if (!viewingcontents) {
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
        if (externallyviewable) {
          if (!pid) {
            pid = global->getExternalFileViewing()->viewThenDelete(path);
          }
          ui->printStr(1, 1, "Opening " + file + " with: " + global->getExternalFileViewing()->getViewApplication(file));
          ui->printStr(3, 1, "Press 'k' to kill this external viewer instance.");
          ui->printStr(4, 1, "You can always press 'K' to kill ALL external viewers.");
        }
        else {
          binary_data tmpdata = global->getLocalStorage()->getFileContent(file);
          global->getLocalStorage()->deleteFile(file);
          std::string extension = ExternalFileViewing::getExtension(file);
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
          viewingcontents = true;
          autoupdate = false;
          redraw();
        }
        break;
    }
  }
  else {
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

    unsigned int slidersize = 0;
    unsigned int sliderstart = 0;
    if (ymax > row) {
      slidersize = (row * row) / ymax;
      sliderstart = (row * y) / ymax;
      if (slidersize == 0) {
        slidersize++;
      }
      if (slidersize == row) {
        slidersize--;
      }
      if (sliderstart + slidersize > row || y + row >= ymax) {
        sliderstart = row - slidersize;
      }
      for (unsigned int i = 0; i < row; i++) {
        if (i >= sliderstart && i < sliderstart + slidersize) {
          ui->printChar(i, col - 1, ' ', true);
        }
        else {
          ui->printChar(i, col - 1, BOX_VLINE);
        }
      }
    }
  }
}

void ViewFileScreen::update() {
  if (download) {
    if (pid) {
      if (!global->getExternalFileViewing()->stillViewing(pid)) {
        ui->returnToLast();
        return;
      }
      else if (!legendupdated) {
        legendupdated = true;
        ui->update();
        ui->setLegend();
        return;
      }
    }
    else if (!pid) {
      redraw();
      if (ts->getState() != TRANSFERSTATUS_STATE_IN_PROGRESS && !legendupdated) {
        legendupdated = true;
        ui->update();
        ui->setLegend();
      }
      return;
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
      if (viewingcontents) {
        if (encoding == encoding::ENCODING_CP437) {
          encoding = encoding::ENCODING_CP437_DOUBLE;
        }
        else if (encoding == encoding::ENCODING_CP437_DOUBLE) {
          encoding = encoding::ENCODING_ISO88591;
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

std::string ViewFileScreen::getLegendText() const {
  if (pid) {
    return "[Esc/Enter/c] Return - [k]ill external viewer - [K]ill ALL external viewers";
  }
  if (!download || !viewingcontents) {
    return "[Esc/Enter/c] Return";
  }
  return "[Arrowkeys] Navigate - [Esc/Enter/c] Return - switch [e]ncoding";
}

std::string ViewFileScreen::getInfoLabel() const {
  return "VIEW FILE: " + file;
}

std::string ViewFileScreen::getInfoText() const {
  if (viewingcontents) {
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
    else {
      current = encoding::toUnicode(rawcontents[i]);
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
  std::string speed = util::parseSize(ts->getSpeed() * SIZEPOWER) + "/s";
  int progresspercent = ts->getProgress();
  std::string progress = util::int2Str(progresspercent) + "%";
  std::string timeremaining = util::simpleTimeFormat(ts->getTimeRemaining());
  std::string transferred = util::parseSize(ts->targetSize()) + " / " +
      util::parseSize(ts->sourceSize());
  unsigned int y = 3;
  MenuSelectOption table;
  Pointer<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;
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
  msotb = table.addTextButtonNoContent(y, 1, "transferred", transferred);
  msal->addElement(msotb, 4, RESIZE_CUTEND);
  msotb = table.addTextButtonNoContent(y, 10, "filename", ts->getFile());
  msal->addElement(msotb, 2, RESIZE_WITHLAST3);
  msotb = table.addTextButtonNoContent(y, 60, "remaining", timeremaining);
  msal->addElement(msotb, 5, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 40, "speed", speed);
  msal->addElement(msotb, 6, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, 50, "progress", progress);
  msal->addElement(msotb, 7, RESIZE_REMOVE);
  table.adjustLines(col - 3);
  for (unsigned int i = 0; i < table.size(); i++) {
    Pointer<ResizableElement> re = table.getElement(i);
    if (re->isVisible()) {
      if (re->getIdentifier() == "filename") {
        std::string labeltext = re->getLabelText();
        bool highlight = table.getLineIndex(table.getAdjustableLine(re)) == 1;
        int charswithhighlight = highlight ? labeltext.length() * progresspercent / 100 : 0;
        ui->printStr(re->getRow(), re->getCol(), labeltext.substr(0, charswithhighlight), true);
        ui->printStr(re->getRow(), re->getCol() + charswithhighlight, labeltext.substr(charswithhighlight));
      }
      else {
        ui->printStr(re->getRow(), re->getCol(), re->getLabelText());
      }
    }
  }
}
