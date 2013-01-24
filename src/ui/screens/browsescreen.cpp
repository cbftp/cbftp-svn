#include "browsescreen.h"

BrowseScreen::BrowseScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  sitethread = global->getSiteThreadManager()->getSiteThread(uicommunicator->getArg1());
  site = sitethread->getSite();
  uicommunicator->expectBackendPush();
  requestid = sitethread->requestFileList("/");
  virgin = true;
  resort = false;
  currentviewspan = 0;
  sortmethod = 0;
  list = UIFileList();
  global->updateTime();
  init(window, row, col);
}

void BrowseScreen::redraw() {
  werase(window);
  if (requestid >= 0 && sitethread->requestReady(requestid)) {
    if (!virgin) {
      if (list.cursoredFile() != NULL) {
        selectionhistory.push_front(StringPair(list.getPath(), list.cursoredFile()->getName()));
      }
    }
    virgin = false;
    FileList * filelist = sitethread->getFileList(requestid);
    sitethread->finishRequest(requestid);
    requestid = -1;
    list.parse(filelist);
    sort();
    currentviewspan = 0;
    std::string path = list.getPath();
    for (std::list<StringPair>::iterator it = selectionhistory.begin(); it != selectionhistory.end(); it++) {
      if (it->getKey() == path) {
        list.selectFileName(it->getValue());
        selectionhistory.erase(it);
        break;
      }
    }
    //delete filelist;
  }
  if (resort == true) sort();
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
  if (currentviewspan + row >= list.size() && list.size() + 1 >= row) {
    currentviewspan = list.size() + 1 - row;
    if (currentviewspan > position) {
      currentviewspan = position;
    }
  }
  if (virgin) {
    TermInt::printStr(window, 1, 1, "Let's browse this shit!");
    TermInt::printStr(window, 2, 1, "Fetching list...");
  }
  update();
}

void BrowseScreen::update() {
  if (requestid >= 0 && sitethread->requestReady(requestid)) {
    uicommunicator->newCommand("redraw");
    return;
  }
  std::vector<UIFile *> * uilist = list.getSortedList();
  int maxnamelen;
  for (unsigned int i = 0; i < uilist->size(); i++) {
    int len = (*uilist)[i]->getName().length();
    if (len > maxnamelen) {
      maxnamelen = len;
    }
  }
  unsigned int sizelen = 9;
  unsigned int timelen = 18;
  unsigned int ownerlen = 18;
  bool printsize = false;
  bool printlastmodified = false;
  bool printowner = false;
  int namelimit = 0;
  if (col > 60 + sizelen + timelen + ownerlen + 3) {
    namelimit = col - sizelen - timelen - ownerlen - 3 - 1 - 2;
    printsize = true;
    printlastmodified = true;
    printowner = true;
  }
  else if (col > 60 + sizelen + timelen + 2) {
    namelimit = col - sizelen - timelen - 2 - 1 - 2;
    printsize = true;
    printlastmodified = true;
  }
  else if (col > 40 + sizelen + 1) {
    namelimit = col - sizelen - 1 - 1 - 2;
    printsize = true;
  }
  else {
    namelimit = col - 1;
  }
  unsigned int sizepos = col - sizelen - (printowner ? ownerlen + 1 : 0) - (printlastmodified ? timelen + 1: 0);
  unsigned int timepos = col - timelen - (printowner ? ownerlen : 0);
  for (unsigned int i = 0; i + currentviewspan < uilist->size() && i < row; i++) {
    unsigned int listi = i + currentviewspan;
    bool selected = (*uilist)[listi] == list.cursoredFile();
    if ((*uilist)[listi]->isDirectory()) {
      TermInt::printStr(window, i, 1, "#");
    }
    if (selected) {
      wattron(window, A_REVERSE);
    }
    TermInt::printStr(window, i, 3, (*uilist)[listi]->getName(), namelimit);
    if (selected) {
      wattroff(window, A_REVERSE);
    }
    if (printsize) {
      TermInt::printStr(window, i, sizepos, (*uilist)[listi]->getSizeRepr(), sizelen, true);
      if (printlastmodified) {
        TermInt::printStr(window, i, timepos, (*uilist)[listi]->getLastModified(), timelen);
        if (printowner) {
          TermInt::printStr(window, i, col-ownerlen, (*uilist)[listi]->getOwner() + "/" + (*uilist)[listi]->getGroup(), ownerlen-1);
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
  unsigned int position;
  switch (ch) {
    case 'c':
      uicommunicator->newCommand("return");
      break;
    case 'r':
      //start a race of the selected dir, do nothing if a file is selected
      if (list.cursoredFile() != NULL && list.cursoredFile()->isDirectory()) {
        std::list<std::string> sections = site->getSectionsForPath(list.getPath());
        if (sections.size() > 0) {
          std::string sectionstring;
          for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++) {
            sectionstring += *it + ";";
          }
          sectionstring = sectionstring.substr(0, sectionstring.length() - 1);
          uicommunicator->newCommand("newrace", site->getName(), sectionstring, list.cursoredFile()->getName());
        }
      }
      break;
    case 'b':
      uicommunicator->newCommand("addsection", site->getName(), list.getPath());
      break;
    case 'v':
      //view selected file, do nothing if a directory is selected
      break;
    case 's':
      sortmethod++;
      resort = true;
      uicommunicator->newCommand("redraw");
      break;
    case KEY_RIGHT:
    case 10:
      if (list.cursoredFile() != NULL && list.cursoredFile()->isDirectory()) {
        oldpath = list.getPath();
        if (oldpath.length() > 1) {
          oldpath += "/";
        }
        requestid = sitethread->requestFileList(oldpath + list.cursoredFile()->getName());
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_LEFT:
    case 8:
    case KEY_BACKSPACE:
      oldpath = list.getPath();
      if (oldpath == "") {
        break;
      }
      if (oldpath == "/" ) {
        uicommunicator->newCommand("return");
        break;
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
  return "[c]ancel - [Enter/Right] open dir - [Backspace/Left] return - [r]ace - [v]iew file - [b]ind to section - [s]ort";
}

void BrowseScreen::sort() {
  switch (sortmethod % 9) {
    case 0:
      list.sortCombined();
      break;
    case 1:
      list.sortName(true);
      break;
    case 2:
      list.sortName(false);
      break;
    case 3:
      list.sortSize(false);
      break;
    case 4:
      list.sortSize(true);
      break;
    case 5:
      list.sortTime(false);
      break;
    case 6:
      list.sortTime(true);
      break;
    case 7:
      list.sortOwner(true);
      break;
    case 8:
      list.sortOwner(false);
      break;
  }
  resort = false;
}
