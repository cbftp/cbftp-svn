#include "browsescreen.h"

BrowseScreen::BrowseScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  sitelogic = global->getSiteLogicManager()->getSiteLogic(uicommunicator->getArg1());
  site = sitelogic->getSite();
  uicommunicator->expectBackendPush();
  requestedpath = site->getBasePath();
  requestid = sitelogic->requestFileList(requestedpath);
  virgin = true;
  resort = false;
  changedsort = false;
  currentviewspan = 0;
  sortmethod = 0;
  slidersize = 0;
  sliderstart = 0;
  spinnerpos = 0;
  list = UIFileList();
  global->updateTime();
  init(window, row, col);
}

void BrowseScreen::redraw() {
  werase(window);
  curs_set(0);
  if (requestid >= 0 && sitelogic->requestReady(requestid)) {
    if (!virgin) {
      if (list.cursoredFile() != NULL) {
        selectionhistory.push_front(StringPair(list.getPath(), list.cursoredFile()->getName()));
      }
    }
    virgin = false;
    FileList * filelist = sitelogic->getFileList(requestid);
    requestid = -1;
    list.parse(filelist);
    sitelogic->finishRequest(requestid);
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
  if (list.size() <= row) {
    slidersize = 0;
  }
  else {
    slidersize = (row * row) / list.size();
    sliderstart = (row * currentviewspan) / list.size();
    if (slidersize == 0) {
      slidersize++;
    }
    if (slidersize == row) {
      slidersize--;
    }
    if (sliderstart + slidersize > row || currentviewspan + row >= list.size()) {
      sliderstart = row - slidersize;
    }
  }
  if (virgin) {
    TermInt::printStr(window, 1, 1, "Getting file list for " + site->getName() + " ...");
  }
  else if (list.size() == 0) {
    TermInt::printStr(window, 0, 3, "(empty directory)");
  }
  update();
}

void BrowseScreen::update() {
  if (requestid >= 0 && sitelogic->requestReady(requestid)) {
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
    std::string prepchar = " ";
    bool allowed = global->getSkipList()->isAllowed((*uilist)[listi]->getName());
    if ((*uilist)[listi]->isDirectory()) {
      if (allowed) {
        prepchar = "#";
      }
      else {
        prepchar = "S";
      }
    }
    else if ((*uilist)[listi]->isLink()){
      prepchar = "L";
    }
    TermInt::printStr(window, i, 1, prepchar);
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
  if (slidersize > 0) {
    for (unsigned int i = 0; i < row; i++) {
      if (i >= sliderstart && i < sliderstart + slidersize) {
        wattron(window, A_REVERSE);
        TermInt::printChar(window, i, col-1, ' ');
        wattroff(window, A_REVERSE);
      }
      else {
        TermInt::printChar(window, i, col-1, 4194424);
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
  bool isdir;
  bool islink;
  UIFile * cursoredfile;
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
      changedsort = true;
      tickcount = 0;
      uicommunicator->newCommand("redraw");
      break;
    case 'S':
      sortmethod = 0;
      resort = true;
      changedsort = true;
      tickcount = 0;
      uicommunicator->newCommand("redraw");
      break;
    case KEY_RIGHT:
    case 10:
      cursoredfile = list.cursoredFile();
      if (cursoredfile == NULL) {
        break;
      }
      isdir = cursoredfile->isDirectory();
      islink = cursoredfile->isLink();
      if (isdir || islink) {
        oldpath = list.getPath();
        if (oldpath.length() > 1) {
          oldpath += "/";
        }
        if (islink) {
          std::string target = cursoredfile->getLinkTarget();
          if (target.length() > 0 && target[0] == '/') {
            requestedpath = target;
          }
          else {
            requestedpath = oldpath + target;
          }
        }
        else {
          requestedpath = oldpath + cursoredfile->getName();
        }
        requestid = sitelogic->requestFileList(requestedpath);
        uicommunicator->newCommand("updatesetinfo");
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
      requestedpath = oldpath.substr(0, position);
      requestid = sitelogic->requestFileList(requestedpath);
      uicommunicator->newCommand("updatesetinfo");
      //go up one directory level, or return if at top already
      break;
    case KEY_F(5):
      requestedpath = list.getPath();
      requestid = sitelogic->requestFileList(requestedpath);
      uicommunicator->newCommand("updatesetinfo");
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
    case KEY_HOME:
      while (list.goPrevious()) {
        if (!update) {
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
    case KEY_END:
      while (list.goNext()) {
        if (!update) {
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
    case 'w':
      uicommunicator->newCommand("rawcommand", site->getName());
      break;
  }
}

std::string BrowseScreen::getLegendText() {
  return "[c]ancel - [Enter/Right] open dir - [Backspace/Left] return - [r]ace - [v]iew file - [b]ind to section - [s]ort - ra[w] command";
}

std::string BrowseScreen::getInfoLabel() {
  return "BROWSING: " + site->getName();
}

std::string BrowseScreen::getInfoText() {
  std::string text = list.getPath();
  if (requestid >= 0) {
    text = "Getting list for " + requestedpath + "  ";
    switch(spinnerpos++ % 4) {
      case 0:
        text += "|";
        break;
      case 1:
        text += "/";
        break;
      case 2:
        text += "-";
        break;
      case 3:
        text += "\\";
        break;
    }
    return text;
  }
  if (changedsort) {
    if (tickcount++ < 8) {
      text = "Sort method: " + list.getSortMethod();
      return text;
    }
    else {
      changedsort = false;
    }
  }
  text += "  " + global->int2Str(list.sizeFiles()) + "f " + global->int2Str(list.sizeDirs()) + "d";
  text += std::string("  ") + UIFile::parseSize(list.getTotalSize());
  return text;
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
