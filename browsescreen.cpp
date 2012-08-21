#include "browsescreen.h"

BrowseScreen::BrowseScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  sitethread = global->getSiteThreadManager()->getSiteThread(uicommunicator->getArg1());
  site = sitethread->getSite();
  uicommunicator->expectBackendPush();
  requestid = sitethread->requestFileList("/");
  virgin = true;
  currentviewspan = 0;
  init(window, row, col);
}

void BrowseScreen::redraw() {
  werase(window);
  unsigned int position = list.currentCursorPosition();
  unsigned int pagerows = (unsigned int) row / 2;
  if (position < currentviewspan || position >= currentviewspan + row) {
    if (position < pagerows) {
      currentviewspan = 0;
    }
    else {
      currentviewspan = position - pagerows;
    }
  }
  if (currentviewspan + row > list.size()) {
    currentviewspan = list.size() - row + 1;
  }
  if (virgin) {
    TermInt::printStr(window, 1, 1, "Let's browse this shit!");
    TermInt::printStr(window, 2, 1, "Fetching list...");
  }
  update();
}

void BrowseScreen::update() {
  if (sitethread->requestReady(requestid)) {
    virgin = false;
    FileList * filelist = sitethread->getFileList(requestid);
    sitethread->finishRequest(requestid);
    list.parse(filelist);
    currentviewspan = 0;
    //delete filelist;
    werase(window);
  }
  std::vector<UIFile *> * uilist = list.getSortedList();
  int maxnamelen;
  for (unsigned int i = 0; i < uilist->size(); i++) {
    int len = (*uilist)[i]->getName().length();
    if (len > maxnamelen) {
      maxnamelen = len;
    }
  }
  unsigned int sizelen = 8;
  unsigned int timelen = 16;
  unsigned int ownerlen = 15;
  bool printsize = false;
  bool printlastmodified = false;
  bool printowner = false;
  int namelimit = 0;
  if (col > 60 + sizelen + timelen + ownerlen + 3) {
    namelimit = col - sizelen - timelen - ownerlen - 3 - 1;
    printsize = true;
    printlastmodified = true;
    printowner = true;
  }
  else if (col > 60 + sizelen + timelen + 2) {
    namelimit = col - sizelen - timelen - 2 - 1;
    printsize = true;
    printlastmodified = true;
  }
  else if (col > 40 + sizelen + 1) {
    namelimit = col - sizelen - 1 - 1;
    printsize = true;
  }
  else {
    namelimit = col - 1;
  }
  unsigned int sizepos = col - sizelen - (printowner ? ownerlen + 1 : 0) - (printlastmodified ? timelen + 1: 0);
  unsigned int timepos = col - timelen - (printowner ? ownerlen : 0);
  for (unsigned int i = 0; i + currentviewspan < uilist->size() && i < row; i++) {
    unsigned int listi = i + currentviewspan;
    bool selected = listi == list.currentCursorPosition();
    if (selected) {
      wattron(window, A_REVERSE);
    }
    TermInt::printStr(window, i, 1, (*uilist)[listi]->getName(), namelimit);
    if (selected) {
      wattroff(window, A_REVERSE);
    }
    if (printsize) {
      TermInt::printStr(window, i, sizepos, global->int2Str((*uilist)[listi]->getSize()), sizelen);
      if (printlastmodified) {
        TermInt::printStr(window, i, timepos, (*uilist)[listi]->getLastModified(), timelen);
        if (printowner) {
          TermInt::printStr(window, i, col-15, (*uilist)[listi]->getOwner() + "/" + (*uilist)[listi]->getGroup(), ownerlen);
        }
      }
    }
  }
}

void BrowseScreen::keyPressed(unsigned int ch) {
  bool update = false;
  bool success = false;
  unsigned int pagerows = (unsigned int) row * 0.6;
  std::string oldpath;
  bool endswithslash;
  unsigned int position;
  switch (ch) {
    case 'c':
      uicommunicator->newCommand("return");
      break;
    case 'r':
      //start a race of the selected dir, do nothing if a file is selected
      break;
    case 'b':
      //bind selected dir to section, do nothing if a file is selected
      break;
    case 'v':
      //view selected file, do nothing if a directory is selected
      break;
    case KEY_RIGHT:
    case 10:
      if (list.cursoredFile()->isDirectory()) {
        oldpath = list.getPath();
        endswithslash = oldpath.rfind("/") + 1 == oldpath.length();
        requestid = sitethread->requestFileList(oldpath + (endswithslash ? "" : "/") + list.cursoredFile()->getName());
        //enter the selected directory, do nothing if a file is selected
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_LEFT:
    case 8:
    case KEY_BACKSPACE:
      oldpath = list.getPath();
      endswithslash = oldpath.rfind("/") + 1 == oldpath.length();
      if (oldpath == "/") {
        uicommunicator->newCommand("return");
        break;
      }
      if (endswithslash) {
        oldpath = oldpath.substr(0, oldpath.length() - 1);
      }
      position = oldpath.rfind("/");
      if (position == 0) {
        position = 1;
      }
      requestid = sitethread->requestFileList(oldpath.substr(0, position));
      uicommunicator->newCommand("update");
      //go up one directory level, or return if at top already
      break;
    case KEY_DOWN:
      //go down and highlight next item (if not at bottom already)
      update = list.goNext();
      if (list.currentCursorPosition() >= currentviewspan + row) {
        uicommunicator->newCommand("redraw");
      }
      else if (update) {
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_UP:
      //go up and highlight previous item (if not at top already)
      update = list.goPrevious();
      if (list.currentCursorPosition() < currentviewspan) {
        uicommunicator->newCommand("redraw");
      }
      else if (update) {
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        success = list.goNext();
        if (!success) {
          break;
        }
        else if (!update) {
          update = true;
        }
      }
      if (list.currentCursorPosition() >= currentviewspan + row) {
        uicommunicator->newCommand("redraw");
      }
      else if (update) {
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        success = list.goPrevious();
        if (!success) {
          break;
        }
        else if (!update) {
          update = true;
        }
      }
      if (list.currentCursorPosition() < currentviewspan) {
        uicommunicator->newCommand("redraw");
      }
      else if (update) {
        uicommunicator->newCommand("update");
      }
      break;
  }
}

std::string BrowseScreen::getLegendText() {
  return "[c]ancel - [Enter/Right] open dir - [Backspace/Left] return - [r]ace - [v]iew file - [b]ind to section";
}
