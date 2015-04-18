#include "viewfilescreen.h"

#include "../ui.h"
#include "../menuselectoption.h"
#include "../resizableelement.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptiontextbutton.h"

#include "../../globalcontext.h"
#include "../../sitelogicmanager.h"
#include "../../sitelogic.h"
#include "../../transfermanager.h"
#include "../../localstorage.h"
#include "../../filelist.h"
#include "../../file.h"
#include "../../externalfileviewing.h"
#include "../../util.h"

extern GlobalContext * global;

ViewFileScreen::ViewFileScreen(Ui * ui) {
  this->ui = ui;
}

void ViewFileScreen::initialize(unsigned int row, unsigned int col, std::string site, std::string file, FileList * filelist) {
  contents.clear();
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
  if (download) {
    downloadattempted = true;
    ts = global->getTransferManager()->suggestDownload(file, sitelogic,
        filelist, temppath);
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
          char * tmpdata = (char *) malloc(MAXOPENSIZE);
          int tmpdatalen = global->getLocalStorage()->getFileContent(file, tmpdata);
          global->getLocalStorage()->deleteFile(file);
          {
            int last = 0;
            for (int i = 0; i < tmpdatalen; i++) {
              if (tmpdata[i] == '\n') {
                contents.push_back(std::string(tmpdata + last, i - last));
                last = i + 1;
              }
            }
            if (last != tmpdatalen) {
              contents.push_back(std::string(tmpdata + last, tmpdatalen - last));
            }
          }
          delete tmpdata;
          ymax = contents.size();
          for (unsigned int i = 0; i < ymax; i++) {
            if (contents[i].length() > xmax) {
              xmax = contents[i].length();
            }
          }
          viewingcontents = true;
          autoupdate = false;
          redraw();
        }
        break;
    }
  }
  else {
    for (unsigned int i = 0; i < row && i < ymax; i++) {
      ui->printStr(i, 1, contents[y + i], col - 2);
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

void ViewFileScreen::keyPressed(unsigned int ch) {
  switch(ch) {
    case 27: // esc
    case ' ':
    case 'c':
    case 10:
      ui->returnToLast();
      break;
    case KEY_DOWN:
      if (goDown()) {
        goDown();
        ui->redraw();
      }
      break;
    case KEY_UP:
      if (goUp()) {
        goUp();
        ui->redraw();
      }
      break;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < row / 2; i++) {
        goDown();
      }
      ui->redraw();
      break;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < row / 2; i++) {
        goUp();
      }
      ui->redraw();
      break;
    case 'k':
      if (pid) {
        global->getExternalFileViewing()->killProcess(pid);
      }
      break;
  }
}

std::string ViewFileScreen::getLegendText() const {
  if (pid) {
    return "[Esc/Enter/c] Return - [k]ill external viewer - [K]ill ALL external viewers";
  }
  if (!download || !viewingcontents) {
    return "[Esc/Enter/c] Return";
  }
  return "[Arrowkeys] Navigate - [Esc/Enter/c] Return";
}

std::string ViewFileScreen::getInfoLabel() const {
  return "VIEW FILE: " + file;
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
