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
  //requestid = sitelogic->requestViewFile(path, file);
  requestid = global->getTransferManager()->download(file, sitelogic, filelist);
  uicommunicator->expectBackendPush();
  init(window, row, col);
}

void ViewFileScreen::redraw() {
  werase(window);
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
        break;
      case TRANSFER_SUCCESSFUL:
        if (filelist->getFile(file)->getSize() <= MAXOPENSIZE) {
          tmpdata = (char *) malloc(MAXOPENSIZE);
          tmpdatalen = global->getLocalStorage()->getFileContent(file, tmpdata);
          {
            int last = 0;
            for (int i = 0; i < tmpdatalen; i++) {
              if (tmpdata[i] == '\n') {
                contents.push_back(std::string(tmpdata + last, i - last));
                last = i + 1;
              }
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
          redraw();
        }
        else {
          TermInt::printStr(window, 1, 1, "Download of " + file + " from " + site + " successful, but the file is too large to open");
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
  if (global->getTransferManager()->transferStatus(requestid) != TRANSFER_IN_PROGRESS_UI) {
    uicommunicator->newCommand("redraw");
    return;
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
  }
}

std::string ViewFileScreen::getLegendText() {
  return "[Arrowkeys] Navigate - [Esc/Enter/c] Return";
}

std::string ViewFileScreen::getInfoLabel() {
  return "VIEW FILE";
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
