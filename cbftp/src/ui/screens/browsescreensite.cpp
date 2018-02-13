#include "browsescreensite.h"

#include <cctype>
#include <algorithm>

#include "../../core/tickpoke.h"
#include "../../core/pointer.h"
#include "../../sitelogic.h"
#include "../../sitelogicmanager.h"
#include "../../site.h"
#include "../../globalcontext.h"
#include "../../skiplist.h"
#include "../../eventlog.h"
#include "../../filelist.h"
#include "../../engine.h"
#include "../../localstorage.h"
#include "../../util.h"
#include "../../timereference.h"
#include "../../rawbuffer.h"

#include "../ui.h"
#include "../uifile.h"
#include "../termint.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectadjustableline.h"
#include "../resizableelement.h"
#include "../misc.h"

#include "browsescreenaction.h"
#include "rawdatascreen.h"

BrowseScreenSite::BrowseScreenSite(Ui * ui, const std::string & sitestr) {
  this->ui = ui;
  gotomodeticker = 0;
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitestr);
  site = sitelogic->getSite();
  requestedpath = site->getBasePath();
  sitelogic->getAggregatedRawBuffer()->bookmark();
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
  filtermodeinput = false;
  withinraceskiplistreach = false;
  focus = true;
  currentviewspan = 0;
  sortmethod = 0;
  spinnerpos = 0;
  filelist = NULL;
  TimeReference::updateTime();
}

BrowseScreenSite::~BrowseScreenSite() {
  disableGotoMode();
  if (virgin) {
    sitelogic->getAggregatedRawBuffer()->uiWatching(false);
  }
}

BrowseScreenType BrowseScreenSite::type() const {
  return BROWSESCREEN_SITE;
}

void BrowseScreenSite::redraw(unsigned int row, unsigned int col, unsigned int coloffset) {
  row = row - (filtermodeinput ? 2 : 0);
  this->row = row;
  this->col = col;
  this->coloffset = coloffset;
  if (virgin) {
    sitelogic->getAggregatedRawBuffer()->uiWatching(true);
    unsigned int linessincebookmark = sitelogic->getAggregatedRawBuffer()->linesSinceBookmark();
    if (!linessincebookmark) {
      ui->printStr(0, coloffset + 1, "Awaiting slot...");
    }
    else {
      unsigned int linestoprint = linessincebookmark > row ? row : linessincebookmark;
      RawDataScreen::printRawBufferLines(ui, sitelogic->getAggregatedRawBuffer(), linestoprint, col, coloffset);
    }
    return;
  }
  if (resort == true) {
    sort();
  }
  unsigned int position = list.currentCursorPosition();
  unsigned int listsize = list.size();
  adaptViewSpan(currentviewspan, row, position, listsize);

  const std::vector<UIFile *> * uilist = list.getSortedList();
  int maxnamelen = 0;
  for (unsigned int i = 0; i + currentviewspan < uilist->size() && i < row; i++) {
    unsigned int listi = i + currentviewspan;
    if ((*uilist)[listi] != NULL) {
      int len = (*uilist)[listi]->getName().length();
      if (len > maxnamelen) {
        maxnamelen = len;
      }
    }
  }
  separatortext = "";
  while (separatortext.length() < (unsigned int) maxnamelen) {
    separatortext += "-";
  }

  Path prepend = list.getPath();
  if (withinraceskiplistreach) {
    Path subpath = list.getPath() - closestracesectionpath;
    prepend = subpath.cutLevels(-1);
  }

  table.reset();

  for (unsigned int i = 0; i + currentviewspan < uilist->size() && i < row; i++) {
    unsigned int listi = i + currentviewspan;
    UIFile * uifile = (*uilist)[listi];
    if (uifile == NULL) {
      addFileDetails(table, coloffset, i, separatortext);
      continue;
    }
    bool selected = uifile == list.cursoredFile();
    std::string prepchar = " ";
    bool isdir = uifile->isDirectory();
    Path testpath = prepend / uifile->getName();
    bool allowed = withinraceskiplistreach ?
        site->getSkipList().isAllowed((prepend / uifile->getName()).toString(), isdir) :
        site->getSkipList().isAllowed((prepend / uifile->getName()).toString(), isdir, false);
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
    std::string owner = uifile->getOwner() + "/" + uifile->getGroup();
    addFileDetails(table, coloffset, i, prepchar, uifile->getName(), uifile->getSizeRepr(),
        uifile->getLastModified(), owner, true, selected);
  }
  table.adjustLines(col - 3);
  table.checkPointer();
  for (unsigned int i = 0; i < table.size(); i++) {
    Pointer<ResizableElement> re = table.getElement(i);
    bool highlight = false;
    if (table.getSelectionPointer() == i) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight && focus);
    }
  }
  printSlider(ui, row, coloffset + col - 1, listsize, currentviewspan);

  if (listsize == 0) {
    ui->printStr(0, coloffset + 3, "(empty directory)");
  }
  if (filtermodeinput) {
    std::string oldtext = filterfield.getData();
    filterfield = MenuSelectOptionTextField("filter", row + 1, 1, "", oldtext,
        col - 20, 512, false);
    ui->showCursor();
  }
  update();
}

void BrowseScreenSite::update() {
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
      }
      else {
        filelist = newfilelist;
        if (!virgin) {
          if (list.cursoredFile() != NULL) {
            selectionhistory.push_front(std::pair<Path, std::string>(list.getPath(), list.cursoredFile()->getName()));
          }
        }
        else {
          sitelogic->getAggregatedRawBuffer()->uiWatching(false);
          virgin = false;
        }
        unsigned int position = 0;
        bool separatorsenabled = false;
        std::list<std::string> filters;
        std::set<std::string> uniques;
        if (list.getPath() == filelist->getPath()) {
          position = list.currentCursorPosition();
          separatorsenabled = list.separatorsEnabled();
          filters = list.getFilters();
          uniques = list.getUniques();
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
        if (filters.size()) {
          list.setFilters(filters);
        }
        if (uniques.size()) {
          list.setUnique(uniques);
        }
        sort();
        if (position) {
          list.setCursorPosition(position);
        }
        const Path & path = list.getPath();
        for (std::list<std::pair<Path, std::string> >::iterator it = selectionhistory.begin(); it != selectionhistory.end(); it++) {
          if (it->first == path) {
            list.selectFileName(it->second);
            selectionhistory.erase(it);
            break;
          }
        }
        std::list<std::string> sections = site->getSectionsForPartialPath(path);
        int dirlevels = path.split().size();
        withinraceskiplistreach = false;
        int closestsectiondirlevel = -1;
        for (std::list<std::string>::iterator it = sections.begin(); it != sections.end(); it++) {
          Path sectionpath = site->getSectionPath(*it);
          int dirleveldifference = dirlevels - sectionpath.split().size();
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

        //delete filelist;
      }
      ui->redraw();
    }
    ui->setInfo();
    return;
  }
  if (virgin) {
    ui->redraw();
  }
  if (table.size()) {
    Pointer<ResizableElement> re = table.getElement(table.getLastSelectionPointer());
    ui->printStr(re->getRow(), re->getCol(), re->getLabelText());
    re = table.getElement(table.getSelectionPointer());
    ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), focus);
  }
  if (filtermodeinput) {
    std::string pretag = "[Filter(s)]: ";
    ui->printStr(filterfield.getRow(), coloffset + filterfield.getCol(), pretag + filterfield.getContentText());
    ui->moveCursor(filterfield.getRow(), coloffset + filterfield.getCol() + pretag.length() + filterfield.cursorPosition());
  }
}

void BrowseScreenSite::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    if (wipe) {
      requestid = sitelogic->requestWipe(wipetarget, wiperecursive);
    }
    else if (deleting) {
      requestid = sitelogic->requestDelete(wipetarget, deletingrecursive, true, true);
    }
    else {
      global->getEventLog()->log("BrowseScreen", "WARNING: got a 'yes' answer for an unknown command");
    }
    ui->redraw();
    ui->setInfo();
    return;
  }
  else if (command == "no") {
    wipe = false;
    deleting = false;
    ui->redraw();
    return;
  }
  else if (command == "returnnuke") {
    nuking = true;
    requestid = util::str2Int(arg);
    ui->redraw();
    ui->setInfo();
    return;
  }
}

BrowseScreenAction BrowseScreenSite::keyPressed(unsigned int ch) {
  bool update = false;
  bool success = false;
  unsigned int pagerows = (unsigned int) row * 0.6;
  Path oldpath;
  bool isdir;
  bool islink;
  UIFile * cursoredfile;
  if (filtermodeinput) {
    if ((ch >= 32 && ch <= 126) || ch == KEY_BACKSPACE || ch == 8 || ch == 127 ||
        ch == KEY_RIGHT || ch == KEY_LEFT || ch == KEY_DC || ch == KEY_HOME ||
        ch == KEY_END) {
      filterfield.inputChar(ch);
      ui->update();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    }
    else if (ch == 10) {
      std::string filter = filterfield.getData();
      if (filter.length()) {
        list.setFilters(util::split(filter));
        resort = true;
      }
      filtermodeinput = false;
      ui->hideCursor();
      ui->redraw();
      ui->setLegend();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    }
    else if (ch == 27) {
      if (filterfield.getData() != "") {
        filterfield.clear();
        ui->update();
      }
      else {
        filtermodeinput = false;
        ui->hideCursor();
        ui->redraw();
        ui->setLegend();
      }
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    }
  }
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
            break;
          }
        }
      }
      ui->redraw();
      return BrowseScreenAction(BROWSESCREENACTION_CAUGHT);
    }
    else {
      disableGotoMode();
    }
    if (ch == 27) {
      return BrowseScreenAction();
    }
  }
  switch (ch) {
    case 27: // esc
      ui->returnToLast();
      break;
    case 'c':
      return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
    case 'r':
      //start a spread job of the selected dir, do nothing if a file is selected
      if (list.cursoredFile() != NULL && list.cursoredFile()->isDirectory()) {
        std::string dirname = list.cursoredFile()->getName();
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
      if (list.cursoredFile() != NULL && (list.cursoredFile()->isDirectory() || list.cursoredFile()->getSize() > 0)) {
        global->getEngine()->newTransferJobDownload(site->getName(), filelist, list.cursoredFile()->getName(),
                                                    global->getLocalStorage()->getDownloadPath());
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
    case 'P':
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
        if (islink) {
          Path target = cursoredfile->getLinkTarget();
          if (target.isAbsolute()) {
            requestedpath = target;
          }
          else {
            requestedpath = oldpath / target;
          }
        }
        else {
          requestedpath = oldpath / cursoredfile->getName();
        }
        requestid = sitelogic->requestFileList(requestedpath);
        ui->update();
        ui->setInfo();
      }
      return BrowseScreenAction(BROWSESCREENACTION_CHDIR);
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
      wipefile = cursoredfile->getName();
      wipetarget = oldpath / wipefile;
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
      wipefile = cursoredfile->getName();
      wipetarget = oldpath / wipefile;
      ui->goConfirmation("Do you really want to delete " + wipefile);
      break;
    case 'q':
      gotomode = true;
      gotomodefirst = true;
      gotomodeticker = 0;
      gotomodestring = "";
      global->getTickPoke()->startPoke(this, "BrowseScreenSite", 50, 0);
      ui->update();
      ui->setLegend();
      break;
    case 'f':
      if (list.hasFilters()) {
        resort = true;
        list.unsetFilters();
      }
      else {
        filtermodeinput = true;
      }
      ui->redraw();
      ui->setLegend();
      break;
    case KEY_LEFT:
    case 8:
    case 127:
    case KEY_BACKSPACE:
      oldpath = list.getPath();
      if (oldpath == "/") {
        return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
      }
      requestedpath = oldpath.cutLevels(1);
      requestid = sitelogic->requestFileList(requestedpath);
      ui->setInfo();
      //go up one directory level, or return if at top already
      return BrowseScreenAction(BROWSESCREENACTION_CHDIR);
    case KEY_F(5):
      refreshFilelist();
      ui->setInfo();
      break;
    case KEY_DOWN:
      //go down and highlight next item (if not at bottom already)
      update = list.goNext();
      if (list.currentCursorPosition() >= currentviewspan + row) {
        ui->redraw();
      }
      else if (update) {
        table.goDown();
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
        table.goUp();
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
        table.goDown();
      }
      if (update) {
        ui->redraw();
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
        table.goUp();
      }
      if (update) {
        ui->redraw();
      }
      break;
    case KEY_HOME:
      while (list.goPrevious()) {
        table.goUp();
        if (!update) {
          update = true;
        }
      }
      if (update) {
        ui->redraw();
      }
      break;
    case KEY_END:
      while (list.goNext()) {
        table.goDown();
        if (!update) {
          update = true;
        }
      }
      if (update) {
        ui->redraw();
      }
      break;
    case 'w':
      if (list.cursoredFile() != NULL) {
        ui->goRawCommand(site->getName(), list.getPath(), list.cursoredFile()->getName());
      }
      else {
        ui->goRawCommand(site->getName(), list.getPath());
      }
      break;
  }
  return BrowseScreenAction();
}

std::string BrowseScreenSite::getLegendText() const {
  if (gotomode) {
    return "[Any] Go to first matching entry name - [Esc] Cancel";
  }
  if (filtermodeinput) {
    return "[Any] Enter space separated filters. Valid operators are !, *, ?. Must match all negative filters and at least one positive if given. Case insensitive. - [Esc] Cancel";
  }
  return "[Esc] Cancel - [c]lose - [Up/Down] Navigate - [Enter/Right] open dir - [Backspace/Left] return - sp[r]ead - [v]iew file - [D]ownload - [b]ind to section - [s]ort - ra[w] command - [W]ipe - [Del]ete - [n]uke - Toggle se[P]arators - [q]uick jump - Toggle [f]ilter";
}

std::string BrowseScreenSite::getInfoLabel() const {
  return "BROWSING: " + site->getName();
}

std::string BrowseScreenSite::getInfoText() const {
  std::string text = list.getPath().toString();
  if (requestid >= 0) {
    if (wipe) {
      text = "Wiping " + wipetarget.toString() + "  ";
    }
    else if (deleting) {
      text = "Deleting " + wipetarget.toString() + "  ";
    }
    else if (nuking) {
      text = "Nuking " + wipetarget.toString() + "  ";
    }
    else {
      text = "Getting list for " + requestedpath.toString() + "  ";
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
      text = "Wipe successful: " + wipetarget.toString();
      return text;
    }
    else {
      wipesuccess = false;
    }
  }
  else if (wipefailed) {
    if (tickcount++ < 8) {
      text = "Wipe failed: " + wipetarget.toString();
      return text;
    }
    else {
      wipefailed = false;
    }
  }
  else if (cwdfailed) {
    if (tickcount++ < 8) {
      text = "Getting file list failed: " + requestedpath.toString();
      return text;
    }
    else {
      cwdfailed = false;
    }
  }
  else if (deletesuccess) {
    if (tickcount++ < 8) {
      text = "Delete successful: " + wipetarget.toString();
      return text;
    }
    else {
      deletesuccess = false;
    }
  }
  else if (deletefailed) {
    if (tickcount++ < 8) {
      text = "Delete failed: " + wipetarget.toString();
      return text;
    }
    else {
      deletefailed = false;
    }
  }
  else if (nukesuccess) {
    if (tickcount++ < 8) {
      text = "Nuke successful: " + nuketarget.toString();
      return text;
    }
    else {
      nukesuccess = false;
    }
  }
  else if (nukefailed) {
    if (tickcount++ < 8) {
      text = "Nuke failed: " + nuketarget.toString();
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
  if (list.hasFilters() || list.hasUnique()) {
    if (list.hasFilters()) {
      text += std::string("  FILTER: ") + filterfield.getData();
    }
    if (list.hasUnique()) {
      text += "  UNIQUES";
    }
    text += "  " + util::int2Str(list.filteredSizeFiles()) + "/" + util::int2Str(list.sizeFiles()) + "f " +
        util::int2Str(list.filteredSizeDirs()) + "/" + util::int2Str(list.sizeDirs()) + "d";
    text += std::string("  ") + util::parseSize(list.getFilteredTotalSize()) + "/" +
        util::parseSize(list.getTotalSize());
  }
  else {
    text += "  " + util::int2Str(list.sizeFiles()) + "f " + util::int2Str(list.sizeDirs()) + "d";
    text += std::string("  ") + util::parseSize(list.getTotalSize());
  }
  return text;
}

void BrowseScreenSite::setFocus(bool focus) {
  this->focus = focus;
  if (!focus) {
    disableGotoMode();
    filtermodeinput = false;
  }
}

std::string BrowseScreenSite::siteName() const {
  return site->getName();
}

FileList * BrowseScreenSite::fileList() const {
  return filelist;
}

UIFile * BrowseScreenSite::selectedFile() const {
  return list.cursoredFile();
}

void BrowseScreenSite::sort() {
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

void BrowseScreenSite::refreshFilelist() {
  requestedpath = list.getPath();
  requestid = sitelogic->requestFileList(requestedpath);
}

void BrowseScreenSite::tick(int) {
  if (gotomode && !gotomodefirst) {
    if (gotomodeticker++ >= 20) {
      disableGotoMode();
    }
  }
}

void BrowseScreenSite::addFileDetails(MenuSelectOption & table, unsigned int coloffset, unsigned int y, const std::string & name) {
  addFileDetails(table, coloffset, y, "", name, "", "", "", false, false);
}

void BrowseScreenSite::addFileDetails(MenuSelectOption & table, unsigned int coloffset, unsigned int y, const std::string & prepchar, const std::string & name, const std::string & size, const std::string & lastmodified, const std::string & owner, bool selectable, bool selected) {
  Pointer<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  Pointer<MenuSelectOptionTextButton> msotb;
  msotb = table.addTextButtonNoContent(y, coloffset + 1, "prepchar", prepchar);
  msotb->setSelectable(false);
  msotb->setShortSpacing();
  msal->addElement(msotb, 4, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, coloffset + 3, "name", name);
  if (!selectable) {
    msotb->setSelectable(false);
  }
  if (selected) {
    table.setPointer(msotb);
  }
  msal->addElement(msotb, 5, 0, RESIZE_WITHDOTS, true);
  msotb = table.addTextButtonNoContent(y, coloffset + 3, "size", size);
  msotb->setSelectable(false);
  msotb->setRightAligned();
  msal->addElement(msotb, 3, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, coloffset + 3, "lastmodified", lastmodified);
  msotb->setSelectable(false);
  msal->addElement(msotb, 2, RESIZE_REMOVE);
  msotb = table.addTextButtonNoContent(y, coloffset + 3, "owner", owner);
  msotb->setSelectable(false);
  msal->addElement(msotb, 1, RESIZE_REMOVE);
}

void BrowseScreenSite::disableGotoMode() {
  if (gotomode) {
    gotomode = false;
    global->getTickPoke()->stopPoke(this, 0);
    ui->setLegend();
  }
}

UIFileList * BrowseScreenSite::getUIFileList() {
  return &list;
}
