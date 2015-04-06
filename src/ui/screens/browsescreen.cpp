#include "browsescreen.h"

#include <cctype>
#include <algorithm>

#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../site.h"
#include "../../globalcontext.h"
#include "../../skiplist.h"
#include "../../eventlog.h"
#include "../../filelist.h"
#include "../../tickpoke.h"
#include "../../engine.h"
#include "../../localstorage.h"
#include "../../util.h"

#include "../ui.h"
#include "../uifile.h"
#include "../termint.h"

extern GlobalContext * global;

BrowseScreen::BrowseScreen(Ui * ui) {
  this->ui = ui;
  gotomodeticker = 0;
}

void BrowseScreen::initialize(unsigned int row, unsigned int col, std::string sitestr) {
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitestr);
  site = sitelogic->getSite();
  expectbackendpush = true;
  requestedpath = site->getBasePath();
  requestid = sitelogic->requestFileList(requestedpath);
  virgin = true;
  resort = false;
  changedsort = false;
  cwdfailed = false;
  wipe = false;
  wiperecursive = false;
  wipesuccess = false;
  wipefailed = false;
  deleting = false;
  deletingrecursive = false;
  deletesuccess = false;
  deletefailed = false;
  nuking = false;
  nukesuccess = false;
  nukefailed = false;
  gotomode = false;
  withinraceskiplistreach = false;
  split = false;
  currentviewspan = 0;
  sortmethod = 0;
  slidersize = 0;
  sliderstart = 0;
  spinnerpos = 0;
  list = UIFileList();
  global->updateTime();
  init(row, col);
}

void BrowseScreen::redraw() {
  if (requestid >= 0 && sitelogic->requestReady(requestid)) {
    tickcount = 0;
    if (wipe) {
      bool wipestatus = sitelogic->finishRequest(requestid);
      requestid = -1;
      wipe = false;
      if (wipestatus) {
        wipesuccess = true;
        if (list.getPath() == wipepath) {
          refreshFilelist();
        }
      }
      else {
        wipefailed = true;
      }
    }
    else if (deleting) {
      bool deletestatus = sitelogic->finishRequest(requestid);
      requestid = -1;
      deleting = false;
      if (deletestatus) {
        deletesuccess = true;
        if (list.getPath() == wipepath) {
          refreshFilelist();
        }
      }
      else {
        deletefailed = true;
      }
    }
    else if (nuking) {
      bool nukestatus = sitelogic->finishRequest(requestid);
      requestid = -1;
      nuking = false;
      if (nukestatus) {
        nukesuccess = true;
      }
      else {
        nukefailed = true;
      }
      refreshFilelist();
    }
    else {
      FileList * newfilelist = sitelogic->getFileList(requestid);
      if (newfilelist == NULL) {
        cwdfailed = true;
        sitelogic->finishRequest(requestid);
        requestid = -1;
        ui->setInfo();
        ui->update();
        return;
      }
      filelist = newfilelist;
      if (!virgin) {
        if (list.cursoredFile() != NULL) {
          selectionhistory.push_front(std::pair<std::string, std::string>(list.getPath(), list.cursoredFile()->getName()));
        }
      }
      virgin = false;
      unsigned int position = 0;
      bool separatorsenabled = false;
      if (list.getPath() == filelist->getPath()) {
        position = list.currentCursorPosition();
        separatorsenabled = list.separatorsEnabled();
      }
      else {
        currentviewspan = 0;
      }
      list.parse(filelist);
      sitelogic->finishRequest(requestid);
      requestid = -1;
      if (separatorsenabled) {
        list.toggleSeparators();
      }
      sort();
      if (position) {
        list.setCursorPosition(position);
      }
      std::string path = list.getPath();
      for (std::list<std::pair<std::string, std::string> >::iterator it = selectionhistory.begin(); it != selectionhistory.end(); it++) {
        if (it->first == path) {
          list.selectFileName(it->second);
          selectionhistory.erase(it);
          break;
        }
      }
      std::list<std::string> sections = site->getSectionsForPartialPath(path);
      int dirlevels = countDirLevels(path);
      withinraceskiplistreach = false;
      int closestsectiondirlevel = -1;
      for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++) {
        std::string sectionpath = site->getSectionPath(*it);
        int dirleveldifference = dirlevels - countDirLevels(sectionpath);
        if (dirleveldifference > 0) {
          withinraceskiplistreach = true;
          if (dirleveldifference < closestsectiondirlevel || closestsectiondirlevel < 0) {
            closestsectiondirlevel = dirleveldifference;
            closestracesectionpath = sectionpath;
          }
        }
        else {
          withinraceskiplistreach = false;
          break;
        }
      }
      ui->setInfo();
      ui->update();
      //delete filelist;
    }
  }
  ui->erase();
  ui->hideCursor();
  ui->setSplit(split);
  if (resort == true) sort();
  unsigned int position = list.currentCursorPosition();
  unsigned int pagerows = (unsigned int) row / 2;
  unsigned int listsize = list.size();
  if (position < currentviewspan || position >= currentviewspan + row) {
    if (position < pagerows) {
      currentviewspan = 0;
    }
    else {
      currentviewspan = position - pagerows;
    }
  }
  if (currentviewspan + row >= listsize && listsize + 1 >= row) {
    currentviewspan = listsize + 1 - row;
    if (currentviewspan > position) {
      currentviewspan = position;
    }
  }

  if (listsize <= row) {
    slidersize = 0;
  }
  else {
    slidersize = (row * row) / listsize;
    sliderstart = (row * currentviewspan) / listsize;
    if (slidersize == 0) {
      slidersize++;
    }
    if (slidersize == row) {
      slidersize--;
    }
    if (sliderstart + slidersize > row || currentviewspan + row >= listsize) {
      sliderstart = row - slidersize;
    }
  }
  if (virgin) {
    ui->printStr(1, 1, "Getting file list for " + site->getName() + " ...");
  }
  else if (listsize == 0) {
    ui->printStr(0, 3, "(empty directory)");
  }
  if (split) {
    for (unsigned int i = 0; i < row; i++) {
      ui->printChar(i, col / 2, BOX_VLINE);
    }
  }
  update();
}

void BrowseScreen::update() {
  if (requestid >= 0 && sitelogic->requestReady(requestid)) {
    ui->redraw();
    return;
  }
  const std::vector<UIFile *> * uilist = list.getSortedList();
  int maxnamelen = 0;
  for (unsigned int i = 0; i < uilist->size(); i++) {
    if ((*uilist)[i] != NULL) {
      int len = (*uilist)[i]->getName().length();
      if (len > maxnamelen) {
        maxnamelen = len;
      }
    }
  }
  unsigned int sizelen = 9;
  unsigned int timelen = 18;
  unsigned int ownerlen = 18;
  bool printsize = false;
  bool printlastmodified = false;
  bool printowner = false;
  unsigned int namelimit = 0;
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
  std::string separatortext;
  while (separatortext.length() < (unsigned int) maxnamelen) {
    separatortext += "-";
  }
  std::string prepend = list.getPath() + "/";
  if (withinraceskiplistreach) {
    std::string subpathinracepath;
    std::string subpath = list.getPath().substr(closestracesectionpath.length() + 1);
    size_t subdirpos = subpath.find("/");
    if (subdirpos != std::string::npos) {
      subpathinracepath = subpath.substr(subdirpos + 1) + "/";
    }
    prepend = subpathinracepath;
  }
  for (unsigned int i = 0; i + currentviewspan < uilist->size() && i < row; i++) {
    unsigned int listi = i + currentviewspan;
    UIFile * uifile = (*uilist)[listi];
    if (uifile == NULL) {
      ui->printStr(i, 3, separatortext, namelimit);
      continue;
    }
    bool selected = uifile == list.cursoredFile();
    std::string prepchar = " ";
    bool isdir = uifile->isDirectory();
    bool allowed = withinraceskiplistreach ?
        global->getSkipList()->isAllowed(prepend + uifile->getName(), isdir) :
        global->getSkipList()->isAllowed(prepend + uifile->getName(), isdir, false);
    if (isdir) {
      if (allowed) {
        prepchar = "#";
      }
      else {
        prepchar = "S";
      }
    }
    else if (!allowed) {
      prepchar = "s";
    }
    else if (uifile->isLink()){
      prepchar = "L";
    }
    ui->printStr(i, 1, prepchar);
    ui->printStr(i, 3, uifile->getName(), namelimit, selected);
    if (printsize) {
      ui->printStr(i, sizepos, uifile->getSizeRepr(), sizelen, false, true);
      if (printlastmodified) {
        ui->printStr(i, timepos, uifile->getLastModified(), timelen);
        if (printowner) {
          ui->printStr(i, col-ownerlen, uifile->getOwner() + "/" + uifile->getGroup(), ownerlen-1);
        }
      }
    }
  }
  if (slidersize > 0) {
    for (unsigned int i = 0; i < row; i++) {
      if (i >= sliderstart && i < sliderstart + slidersize) {
        ui->printChar(i, col-1, ' ', true);
      }
      else {
        ui->printChar(i, col-1, 4194424);
      }
    }
  }
}

void BrowseScreen::command(std::string command, std::string arg) {
  if (command == "yes") {
    if (wipe) {
      requestid = sitelogic->requestWipe(wipetarget, wiperecursive);
    }
    else if (deleting) {
      requestid = sitelogic->requestDelete(wipetarget, deletingrecursive, true);
    }
    else {
      global->getEventLog()->log("BrowseScreen", "WARNING: got a 'yes' answer for an unknown command");
    }
    redraw();
    ui->setInfo();
    ui->update();
    return;
  }
  else if (command == "no") {
    wipe = false;
    deleting = false;
    redraw();
    return;
  }
  else if (command == "returnnuke") {
    nuking = true;
    requestid = util::str2Int(arg);
    redraw();
    ui->update();
    ui->setInfo();
    return;
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
  if (gotomode) {
    if (gotomodefirst) {
      gotomodefirst = false;
    }
    if (ch >= 32 && ch <= 126) {
      gotomodeticker = 0;
      gotomodestring += toupper(ch);
      unsigned int gotomodelength = gotomodestring.length();
      const std::vector<UIFile *> * sortedlist = list.getSortedList();
      for (unsigned int i = 0; i < sortedlist->size(); i++) {
        if ((*sortedlist)[i] == NULL) continue;
        std::string name = (*sortedlist)[i]->getName();
        if (name.length() >= gotomodelength) {
          std::string substr = name.substr(0, gotomodelength);
          for (unsigned int j = 0; j < gotomodelength; j++) {
            substr[j] = toupper(substr[j]);
          }
          if (substr == gotomodestring) {
            list.setCursorPosition(i);
            if (list.currentCursorPosition() >= currentviewspan + row ||
                list.currentCursorPosition() < currentviewspan) {
              ui->redraw();
            }
            break;
          }
        }
      }
      ui->update();
      return;
    }
    else {
      gotomode = false;
      global->getTickPoke()->stopPoke(this, 0);
      ui->update();
      ui->setLegend();
    }
    if (ch == 27) {
      return;
    }
  }
  switch (ch) {
    case 27: // esc
    case 'c':
      ui->returnToLast();
      break;
    case 'r':
      //start a race of the selected dir, do nothing if a file is selected
      if (list.cursoredFile() != NULL && list.cursoredFile()->isDirectory()) {
        std::string dirname = list.cursoredFile()->getName();
        if (!global->getSkipList()->isAllowed(dirname, true, false)) {
          break;
        }
        std::list<std::string> sections = site->getSectionsForPath(list.getPath());
        if (sections.size() > 0) {
          std::string sectionstring;
          for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++) {
            sectionstring += *it + ";";
          }
          sectionstring = sectionstring.substr(0, sectionstring.length() - 1);
          ui->goNewRace(site->getName(), sectionstring, dirname);
        }
      }
      break;
    case 'b':
      ui->goAddSection(site->getName(), list.getPath());
      break;
    case 'v':
      //view selected file, do nothing if a directory is selected
      if (list.cursoredFile() != NULL && !list.cursoredFile()->isDirectory()) {
        ui->goViewFile(site->getName(), list.cursoredFile()->getName(), filelist);
      }
      break;
    case 'D':
      if (list.cursoredFile()->isDirectory() || list.cursoredFile()->getSize() > 0) {
        global->getEngine()->newTransferJobDownload(site->getName(), list.cursoredFile()->getName(),
            filelist, global->getLocalStorage()->getDownloadPath());
      }
      break;
    case 's':
      sortmethod++;
      resort = true;
      changedsort = true;
      tickcount = 0;
      ui->redraw();
      ui->setInfo();
      break;
    case 'S':
      sortmethod = 0;
      resort = true;
      changedsort = true;
      tickcount = 0;
      ui->redraw();
      ui->setInfo();
      break;
    case 'p':
      resort = true;
      list.toggleSeparators();
      ui->redraw();
      break;
    case 'n':
      if (list.cursoredFile() != NULL && list.cursoredFile()->isDirectory()) {
        tickcount = 0;
        ui->goNuke(site->getName(), list.cursoredFile()->getName(), filelist);
        nuketarget = list.cursoredFile()->getName();
      }
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
        ui->update();
        ui->setInfo();
      }
      break;
    case 'W':
      cursoredfile = list.cursoredFile();
      if (cursoredfile == NULL) {
        break;
      }
      tickcount = 0;
      wipe = true;
      wiperecursive = false;
      if (cursoredfile->isDirectory()) {
        wiperecursive = true;
      }
      oldpath = list.getPath();
      wipepath = oldpath;
      if (oldpath.length() > 1) {
        oldpath += "/";
      }
      wipefile = cursoredfile->getName();
      wipetarget = oldpath + wipefile;
      ui->goConfirmation("Do you really want to wipe " + wipefile);
      break;
    case KEY_DC:
      cursoredfile = list.cursoredFile();
      if (cursoredfile == NULL) {
        break;
      }
      tickcount = 0;
      deleting = true;
      deletingrecursive = false;
      if (cursoredfile->isDirectory()) {
        deletingrecursive = true;
      }
      oldpath = list.getPath();
      wipepath = oldpath;
      if (oldpath.length() > 1) {
        oldpath += "/";
      }
      wipefile = cursoredfile->getName();
      wipetarget = oldpath + wipefile;
      ui->goConfirmation("Do you really want to delete " + wipefile);
      break;
    case 'q':
      gotomode = true;
      gotomodefirst = true;
      gotomodeticker = 0;
      gotomodestring = "";
      global->getTickPoke()->startPoke(this, "BrowseScreen", 50, 0);
      ui->update();
      ui->setLegend();
      break;
    case KEY_LEFT:
    case 8:
    case 127:
    case KEY_BACKSPACE:
      oldpath = list.getPath();
      if (oldpath == "") {
        break;
      }
      if (oldpath == "/" ) {
        ui->returnToLast();
        break;
      }
      position = oldpath.rfind("/");
      if (position == 0) {
        position = 1;
      }
      requestedpath = oldpath.substr(0, position);
      requestid = sitelogic->requestFileList(requestedpath);
      ui->update();
      ui->setInfo();
      //go up one directory level, or return if at top already
      break;
    case KEY_F(5):
      refreshFilelist();
      ui->update();
      ui->setInfo();
      break;
    case KEY_DOWN:
      //go down and highlight next item (if not at bottom already)
      update = list.goNext();
      if (list.currentCursorPosition() >= currentviewspan + row) {
        ui->redraw();
      }
      else if (update) {
        ui->update();
      }
      break;
    case KEY_UP:
      //go up and highlight previous item (if not at top already)
      update = list.goPrevious();
      if (list.currentCursorPosition() < currentviewspan) {
        ui->redraw();
      }
      else if (update) {
        ui->update();
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
        ui->redraw();
      }
      else if (update) {
        ui->update();
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
        ui->redraw();
      }
      else if (update) {
        ui->update();
      }
      break;
    case KEY_HOME:
      while (list.goPrevious()) {
        if (!update) {
          update = true;
        }
      }
      if (list.currentCursorPosition() < currentviewspan) {
        ui->redraw();
      }
      else if (update) {
        ui->update();
      }
      break;
    case KEY_END:
      while (list.goNext()) {
        if (!update) {
          update = true;
        }
      }
      if (list.currentCursorPosition() >= currentviewspan + row) {
        ui->redraw();
      }
      else if (update) {
        ui->update();
      }
      break;
    case 'w':
      if (list.cursoredFile() != NULL) {
        ui->goRawCommand(site->getName(), list.cursoredFile()->getName());
      }
      else {
        ui->goRawCommand(site->getName());
      }
      break;
    case '\t':
      if (!split) {
        split = true;
        ui->redraw();
      }
      else {
        ui->update();
      }

      break;
  }
}

std::string BrowseScreen::getLegendText() const {
  if (gotomode) {
    return "[Any] Go to first matching entry name - [Esc] Cancel";
  }
  return "[c]ancel - [Enter/Right] open dir - [Backspace/Left] return - [r]ace - [v]iew file - [D]ownload - [b]ind to section - [s]ort - ra[w] command - [W]ipe - [Del]ete - [n]uke - Toggle se[p]arators - [q]uick jump";
}

std::string BrowseScreen::getInfoLabel() const {
  return "BROWSING: " + site->getName();
}

std::string BrowseScreen::getInfoText() const {
  std::string text = list.getPath();
  if (requestid >= 0) {
    if (wipe) {
      text = "Wiping " + wipetarget + "  ";
    }
    else if (deleting) {
      text = "Deleting " + wipetarget + "  ";
    }
    else if (nuking) {
      text = "Nuking " + wipetarget + "  ";
    }
    else {
      text = "Getting list for " + requestedpath + "  ";
    }
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
  if (wipesuccess) {
    if (tickcount++ < 8) {
      text = "Wipe successful: " + wipetarget;
      return text;
    }
    else {
      wipesuccess = false;
    }
  }
  else if (wipefailed) {
    if (tickcount++ < 8) {
      text = "Wipe failed: " + wipetarget;
      return text;
    }
    else {
      wipefailed = false;
    }
  }
  else if (cwdfailed) {
    if (tickcount++ < 8) {
      text = "CWD failed: " + requestedpath;
      return text;
    }
    else {
      cwdfailed = false;
    }
  }
  else if (deletesuccess) {
    if (tickcount++ < 8) {
      text = "Delete successful: " + wipetarget;
      return text;
    }
    else {
      deletesuccess = false;
    }
  }
  else if (deletefailed) {
    if (tickcount++ < 8) {
      text = "Delete failed: " + wipetarget;
      return text;
    }
    else {
      deletefailed = false;
    }
  }
  else if (nukesuccess) {
    if (tickcount++ < 8) {
      text = "Nuke successful: " + nuketarget;
      return text;
    }
    else {
      nukesuccess = false;
    }
  }
  else if (nukefailed) {
    if (tickcount++ < 8) {
      text = "Nuke failed: " + nuketarget;
      return text;
    }
    else {
      nukefailed = false;
    }
  }
  else if (changedsort) {
    if (tickcount++ < 8) {
      text = "Sort method: " + list.getSortMethod();
      return text;
    }
    else {
      changedsort = false;
    }
  }
  text += "  " + util::int2Str(list.sizeFiles()) + "f " + util::int2Str(list.sizeDirs()) + "d";
  text += std::string("  ") + util::parseSize(list.getTotalSize());
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

void BrowseScreen::refreshFilelist() {
  requestedpath = list.getPath();
  requestid = sitelogic->requestFileList(requestedpath);
}

void BrowseScreen::tick (int message) {
  if (gotomode && !gotomodefirst) {
    if (gotomodeticker++ >= 20) {
      gotomode = false;
      global->getTickPoke()->stopPoke(this, 0);
      ui->update();
      ui->setLegend();
    }
  }
}

size_t BrowseScreen::countDirLevels(std::string path) {
  if (path.length() == 0) {
    path = "/";
  }
  bool endswithslash = path.rfind("/") + 1 == path.length();
  if (endswithslash && path.length() > 1) {
    path = path.substr(0, path.length() - 1);
  }
  return std::count(path.begin(), path.end(), '/');
}
