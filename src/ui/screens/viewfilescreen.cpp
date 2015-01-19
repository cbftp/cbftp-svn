#include "viewfilescreen.h"

#include "../ui.h"

#include "../../globalcontext.h"
#include "../../sitelogicmanager.h"
#include "../../sitelogic.h"
#include "../../transfermanager.h"
#include "../../localstorage.h"
#include "../../filelist.h"
#include "../../file.h"
#include "../../externalfileviewing.h"

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
  pid = 0;
  autoupdate = true;
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
    requestid = global->getTransferManager()->download(file, sitelogic,
        filelist, temppath);
  }
  path = temppath + "/" + file;
  expectbackendpush = true;
  init(row, col);
}

void ViewFileScreen::redraw() {
  ui->erase();
  if (!download) {
    autoupdate = false;
    if (!externallyviewable) {
      ui->printStr(1, 1, file + " is too large to download and open in the internal viewer.");
      ui->printStr(2, 1, "The maximum file size for internal viewing is set to " + global->int2Str(MAXOPENSIZE) + " bytes.");
    }
    else if (hasnodisplay) {
      ui->printStr(1, 1, file + " cannot be opened in an external viewer.");
      ui->printStr(2, 1, "The DISPLAY environment variable is not set.");
    }
    return;
  }
  if (!viewingcontents) {
    int transferstatus = global->getTransferManager()->transferStatus(requestid);
    char * tmpdata;
    int tmpdatalen;
    switch(transferstatus) {
      case TRANSFER_IN_PROGRESS_UI:
        ui->printStr(1, 1, "Downloading " + file + " from " + site + "...");
        break;
      case TRANSFER_FAILED:
        ui->printStr(1, 1, "Download of " + file + " from " + site + " failed.");
        autoupdate = false;
        break;
      case TRANSFER_SUCCESSFUL:
        if (externallyviewable) {
          if (!pid) {
            pid = global->getExternalFileViewing()->viewThenDelete(path);
          }
          ui->printStr(1, 1, "Opening " + file + " with: " + global->getExternalFileViewing()->getViewApplication(file));
          ui->printStr(3, 1, "Press 'k' to kill this external viewer instance.");
          ui->printStr(4, 1, "You can always press 'K' to kill ALL external viewers.");
        }
        else {
          tmpdata = (char *) malloc(MAXOPENSIZE);
          tmpdatalen = global->getLocalStorage()->getFileContent(file, tmpdata);
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
    else if (!pid && global->getTransferManager()->transferStatus(requestid) != TRANSFER_IN_PROGRESS_UI) {
      redraw();
      if (!legendupdated) {
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
