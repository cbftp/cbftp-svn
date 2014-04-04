#include "viewfilescreen.h"

#include "../uicommunicator.h"
#include "../termint.h"

#include "../../globalcontext.h"
#include "../../sitelogicmanager.h"
#include "../../sitelogic.h"
#include "../../transfermanager.h"
#include "../../localstorage.h"
#include "../../filelist.h"
#include "../../file.h"
#include "../../externalfileviewing.h"

extern GlobalContext * global;

ViewFileScreen::ViewFileScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  contents.clear();
  viewingcontents = false;
  x = 0;
  y = 0;
  ymax = 0;
  xmax = 0;
  site = uicommunicator->getArg1();
  file = uicommunicator->getArg2();
  filelist = (FileList *) uicommunicator->getPointerArg();
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
  if (download) {
    requestid = global->getTransferManager()->download(file, sitelogic, filelist);
  }
  path = global->getLocalStorage()->getTempPath() + "/" + file;
  uicommunicator->expectBackendPush();
  init(window, row, col);
}

void ViewFileScreen::redraw() {
  werase(window);
  if (!download) {
    autoupdate = false;
    if (!externallyviewable) {
      TermInt::printStr(window, 1, 1, file + " is too large to download and open in the internal viewer.");
      TermInt::printStr(window, 2, 1, "The maximum file size for internal viewing is set to " + global->int2Str(MAXOPENSIZE) + " bytes.");
    }
    else if (hasnodisplay) {
      TermInt::printStr(window, 1, 1, file + " cannot be opened in an external viewer.");
      TermInt::printStr(window, 2, 1, "The DISPLAY environment variable is not set.");
    }
    return;
  }
  if (!viewingcontents) {
    int transferstatus = global->getTransferManager()->transferStatus(requestid);
    char * tmpdata;
    int tmpdatalen;
    switch(transferstatus) {
      case TRANSFER_IN_PROGRESS_UI:
        TermInt::printStr(window, 1, 1, "Downloading " + file + " from " + site + "...");
        break;
      case TRANSFER_FAILED:
        TermInt::printStr(window, 1, 1, "Download of " + file + " from " + site + " failed.");
        autoupdate = false;
        break;
      case TRANSFER_SUCCESSFUL:
        if (externallyviewable) {
          if (!pid) {
            pid = global->getExternalFileViewing()->viewThenDelete(path);
          }
          TermInt::printStr(window, 1, 1, "Opening " + file + " with: " + global->getExternalFileViewing()->getViewApplication(file));
          TermInt::printStr(window, 3, 1, "Press 'k' to kill this external viewer instance.");
          TermInt::printStr(window, 4, 1, "You can always press 'K' to kill ALL external viewers.");
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
      TermInt::printStr(window, i, 1, contents[y + i], col - 2);
    }
  }
}

void ViewFileScreen::update() {
  if (download) {
    if (pid) {
      if (!global->getExternalFileViewing()->stillViewing(pid)) {
        uicommunicator->newCommand("return");
        return;
      }
      else if (!legendupdated) {
        uicommunicator->newCommand("updatesetlegend");
        legendupdated = true;
        return;
      }
    }
    else if (!pid && global->getTransferManager()->transferStatus(requestid) != TRANSFER_IN_PROGRESS_UI) {
      redraw();
      if (!legendupdated) {
        uicommunicator->newCommand("updatesetlegend");
        legendupdated = true;
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
      uicommunicator->newCommand("return");
      break;
    case KEY_DOWN:
      if (goDown()) {
        goDown();
        uicommunicator->newCommand("redraw");
      }
      break;
    case KEY_UP:
      if (goUp()) {
        goUp();
        uicommunicator->newCommand("redraw");
      }
      break;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < row / 2; i++) {
        goDown();
      }
      uicommunicator->newCommand("redraw");
      break;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < row / 2; i++) {
        goUp();
      }
      uicommunicator->newCommand("redraw");
      break;
    case 'k':
      if (pid) {
        global->getExternalFileViewing()->killProcess(pid);
      }
      break;
  }
}

std::string ViewFileScreen::getLegendText() {
  if (pid) {
    return "[Esc/Enter/c] Return - [k]ill external viewer - [K]ill ALL external viewers";
  }
  if (!download || !viewingcontents) {
    return "[Esc/Enter/c] Return";
  }
  return "[Arrowkeys] Navigate - [Esc/Enter/c] Return";
}

std::string ViewFileScreen::getInfoLabel() {
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
