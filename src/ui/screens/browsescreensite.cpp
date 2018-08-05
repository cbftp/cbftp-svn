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
#include "../../filelistdata.h"

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
  requestids.push_back(sitelogic->requestFileList(requestedpath));
  virgin = true;
  resort = false;
  changedsort = false;
  cwdfailed = false;
  wipe = false;
  wipesuccess = false;
  wipefailed = false;
  deleting = false;
  deletesuccess = false;
  deletefailed = false;
  nuking = false;
  nukesuccess = false;
  nukefailed = false;
  mkdiring = false;
  mkdirsuccess = false;
  mkdirfailed = false;
  gotomode = false;
  filtermodeinput = false;
  withinraceskiplistreach = false;
  focus = true;
  softselecting = false;
  currentviewspan = 0;
  sortmethod = 0;
  spinnerpos = 0;
  temphighlightline = -1;
  filelist = NULL;
  TimeReference::updateTime();
}

BrowseScreenSite::~BrowseScreenSite() {
  disableGotoMode();
  if (virgin) {
    sitelogic->getAggregatedRawBuffer()->setUiWatching(false);
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
    sitelogic->getAggregatedRawBuffer()->setUiWatching(true);
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
    bool cursored = uifile == list.cursoredFile();
    std::string prepchar = " ";
    bool isdir = uifile->isDirectory();
    Path testpath = prepend / uifile->getName();
    SkipListMatch match = withinraceskiplistreach ?
                          site->getSkipList().check((prepend / uifile->getName()).toString(), isdir) :
                          site->getSkipList().check((prepend / uifile->getName()).toString(), isdir, false);
    bool allowed = !(match.action == SKIPLIST_DENY ||
                    (match.action == SKIPLIST_UNIQUE &&
                     filelist->containsPatternBefore(match.matchpattern, isdir, uifile->getName())));
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
        uifile->getLastModified(), owner, true, cursored, uifile);
  }
  table.adjustLines(col - 3);
  table.checkPointer();
  if (temphighlightline != -1) {
    Pointer<MenuSelectAdjustableLine> highlightline = table.getAdjustableLineOnRow(temphighlightline);
    if (!!highlightline) {
      std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol();
      for (unsigned int i = minmaxcol.first; i <= minmaxcol.second; i++) {
        ui->printChar(temphighlightline, i, ' ', true);
      }
    }
  }
  for (unsigned int i = 0; i < table.size(); i++) {
    Pointer<ResizableElement> re = table.getElement(i);
    bool highlight = false;
    bool cursored = table.getSelectionPointer() == i;
    bool softselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isSoftSelected();
    bool hardselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isHardSelected();
    if (cursored || (int)re->getRow() == temphighlightline || softselected || hardselected)
    {
      highlight = true;
    }
    if (re->isVisible()) {
      if ((cursored && softselected) || (cursored && hardselected) || (softselected && hardselected)) {
        printFlipped(re);
      }
      else {
        ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight && focus);
      }
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
  if (handleReadyRequests()) {
    return;
  }
  if (virgin) {
    ui->redraw();
  }
  if (table.size()) {
    Pointer<ResizableElement> re = table.getElement(table.getLastSelectionPointer());
    bool softselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isSoftSelected();
    bool hardselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isHardSelected();
    if (softselected && hardselected) {
      printFlipped(re);
    }
    else {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), softselected || hardselected);
    }
    re = table.getElement(table.getSelectionPointer());
    bool selected = re->getOrigin() && (static_cast<UIFile *>(re->getOrigin())->isSoftSelected() ||
                                        static_cast<UIFile *>(re->getOrigin())->isHardSelected());
    if (selected && focus) {
      printFlipped(re);
    }
    else {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), focus);
    }
  }
  if (filtermodeinput) {
    std::string pretag = "[Filter(s)]: ";
    ui->printStr(filterfield.getRow(), coloffset + filterfield.getCol(), pretag + filterfield.getContentText());
    ui->moveCursor(filterfield.getRow(), coloffset + filterfield.getCol() + pretag.length() + filterfield.cursorPosition());
  }
}

bool BrowseScreenSite::handleReadyRequests() {
  tickcount = 0;
  bool redraw = false;
  bool handled = false;
  while (!requestids.empty() && sitelogic->requestReady(requestids.front())) {
    handled = true;
    if (wipe) {
      bool wipestatus = sitelogic->finishRequest(requestids.front());
      requestids.pop_front();
      if (requestids.empty()) {
        wipe = false;
        if (wipestatus) {
          wipesuccess = true;
          if (list.getPath() == actionpath) {
            refreshFilelist();
          }
        }
        else {
          wipefailed = true;
        }
      }
    }
    else if (deleting) {
      bool deletestatus = sitelogic->finishRequest(requestids.front());
      requestids.pop_front();
      if (requestids.empty()) {
        deleting = false;
        if (deletestatus) {
          deletesuccess = true;
          if (list.getPath() == actionpath) {
            refreshFilelist();
          }
        }
        else {
          deletefailed = true;
        }
      }
    }
    else if (nuking) {
      bool nukestatus = sitelogic->finishRequest(requestids.front());
      requestids.pop_front();
      if (requestids.empty()) {
        nuking = false;
        if (nukestatus) {
          nukesuccess = true;
        }
        else {
          nukefailed = true;
        }
        refreshFilelist();
      }
    }
    else if (mkdiring) {
      bool mkdirstatus = sitelogic->finishRequest(requestids.front());
      requestids.pop_front();
      if (requestids.empty()) {
        mkdiring = false;
        if (mkdirstatus) {
          mkdirsuccess = true;
          refreshFilelist();
        }
        else {
          mkdirfailed = true;
        }
      }
    }
    else {
      FileListData * newfilelistdata = sitelogic->getFileListData(requestids.front());
      if (newfilelistdata == NULL) {
        sitelogic->finishRequest(requestids.front());
        requestids.pop_front();
        if (requestids.empty()) {
          cwdfailed = true;
        }
      }
      else {
        cwdrawbuffer = newfilelistdata->getCwdRawBuffer();
        filelist = newfilelistdata->getFileList();
        if (!virgin) {
          if (list.cursoredFile() != NULL) {
            selectionhistory.push_front(std::pair<Path, std::string>(list.getPath(), list.cursoredFile()->getName()));
          }
        }
        else {
          sitelogic->getAggregatedRawBuffer()->setUiWatching(false);
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
        sitelogic->finishRequest(requestids.front());
        requestids.pop_front();
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
      redraw = true;
    }
  }
  if (redraw) {
    ui->redraw();
  }
  ui->setInfo();
  return handled;
}
void BrowseScreenSite::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    if (wipe) {
      std::list<std::pair<std::string, bool> >::iterator it;
      for (it = actionfiles.begin(); it != actionfiles.end(); it++) {
        requestids.push_back(sitelogic->requestWipe(actionpath / it->first, it->second));
      }
    }
    else if (deleting) {
      std::list<std::pair<std::string, bool> >::iterator it;
      for (it = actionfiles.begin(); it != actionfiles.end(); it++) {
        requestids.push_back(sitelogic->requestDelete(actionpath / it->first, it->second, true, true));
      }
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
  else if (command == "makedir") {
    mkdiring = true;
    requestids.push_back(sitelogic->requestMakeDirectory(arg));
    actionfiles.push_back(std::pair<std::string, bool>(arg, true));
    ui->redraw();
    ui->setInfo();
    return;
  }
}

void BrowseScreenSite::command(const std::string & command, const std::list<int> & reqids) {
  if (command == "returnnuke") {
    nuking = true;
    for (std::list<int>::const_iterator it = reqids.begin(); it != reqids.end(); it++) {
      requestids.push_back(*it);
    }
    ui->redraw();
    ui->setInfo();
    return;
  }
}

BrowseScreenAction BrowseScreenSite::keyPressed(unsigned int ch) {
  if (temphighlightline != -1) {
    temphighlightline = -1;
    ui->redraw();
    if (ch == '-') {
      return BrowseScreenAction();
    }
  }
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
        clearSoftSelects();
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
            clearSoftSelects();
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
      if (list.clearSelected()) {
        ui->redraw();
        break;
      }
      ui->returnToLast();
      break;
    case 'c':
      return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
    case 'r': {
      //start a spread job of the selected dir, do nothing if a file is selected
      std::list<std::pair<std::string, bool> > items = list.getSelectedDirectoryNames();
      if (items.empty()) {
        break;
      }
      std::list<std::string> sections = site->getSectionsForPath(list.getPath());
      if (sections.empty()) {
        break;
      }
      ui->goNewRace(site->getName(), sections, items);
      break;
    }
    case 'b':
      ui->goAddSection(site->getName(), list.getPath());
      break;
    case 'v':
      //view selected file, do nothing if a directory is selected
      if (list.cursoredFile() != NULL && !list.cursoredFile()->isDirectory()) {
        ui->goViewFile(site->getName(), list.cursoredFile()->getName(), filelist);
      }
      break;
    case 'D': {
      const std::list<UIFile *> items = list.getSelectedFiles();
      for (std::list<UIFile *>::const_iterator it = items.begin(); it != items.end(); it++) {
        UIFile * file = *it;
        if (!file->isDirectory() && !file->getSize()) {
          continue;
        }
        unsigned int id = global->getEngine()->newTransferJobDownload(site->getName(), filelist, file->getName(),
                                                                      global->getLocalStorage()->getDownloadPath());
        ui->addTempLegendTransferJob(id);
      }
      break;
    }
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
      tickcount = 0;
      actionfiles = list.getSelectedDirectoryNames();
      if (actionfiles.empty()) {
        break;
      }
      ui->goNuke(site->getName(), actionfiles, list.getPath());
      break;
    case 'm':
      ui->goMakeDir(site->getName(), list);
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
        requestids.push_back(sitelogic->requestFileList(requestedpath));
        ui->update();
        ui->setInfo();
      }
      return BrowseScreenAction(BROWSESCREENACTION_CHDIR);
    case 'W': {
      std::list<std::pair<std::string, bool> > filenames = list.getSelectedNames();
      if (filenames.empty()) {
        break;
      }
      tickcount = 0;
      wipe = true;
      actionpath = list.getPath();
      actionfiles = filenames;
      std::string targettext = "these " + util::int2Str(static_cast<int>(filenames.size())) + " items";
      if (filenames.size() == 1) {
        targettext = filenames.front().first;
      }
      ui->goConfirmation("Do you really want to wipe " + targettext);
      break;
    }
    case KEY_DC: {
      std::list<std::pair<std::string, bool> > filenames = list.getSelectedNames();
      if (filenames.empty()) {
        break;
      }
      tickcount = 0;
      deleting = true;
      actionpath = list.getPath();
      actionfiles = filenames;
      std::string targettext = "these " + util::int2Str(static_cast<int>(filenames.size())) + " items";
      if (filenames.size() == 1) {
        targettext = filenames.front().first;
      }
      ui->goConfirmation("Do you really want to delete " + targettext);
      break;
    }
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
    case 'l':
      ui->goRawBuffer(&cwdrawbuffer, "CWD LOG: " + site->getName(), list.getPath().toString());
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
      requestids.push_back(sitelogic->requestFileList(requestedpath));
      ui->setInfo();
      //go up one directory level, or return if at top already
      return BrowseScreenAction(BROWSESCREENACTION_CHDIR);
    case KEY_F(5):
      refreshFilelist();
      ui->setInfo();
      break;
    case KEY_DOWN:
      clearSoftSelects();
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
      clearSoftSelects();
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
      clearSoftSelects();
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
      clearSoftSelects();
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
      clearSoftSelects();
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
      clearSoftSelects();
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
    case 'A':
    case TERMINT_CTRL_A: {
      const std::vector<UIFile *> * uilist = list.getSortedList();
      for (unsigned int i = 0; i < uilist->size(); i++) {
        (*uilist)[i]->softSelect();
      }
      softselecting = true;
      ui->redraw();
      break;
    }
    case KEY_SR: // shift up
      if (list.cursoredFile() != NULL) {
        UIFile * lastfile = list.cursoredFile();
        if (!list.goPrevious()) {
          break;
        }
        UIFile * file = list.cursoredFile();
        if (file->isSoftSelected()) {
          file->unSoftSelect();
        }
        else {
          lastfile->softSelect();
          softselecting = true;
        }
        table.goUp();
        ui->redraw();
      }
      break;
    case KEY_SF: // shift down
      if (list.cursoredFile() != NULL) {
        UIFile * lastfile = list.cursoredFile();
        if (!list.goNext()) {
          break;
        }
        UIFile * file = list.cursoredFile();
        if (file->isSoftSelected()) {
          file->unSoftSelect();
        }
        else {
          lastfile->softSelect();
          softselecting = true;
        }
        table.goDown();
        ui->redraw();
      }
      break;
    case ' ':
      if (softselecting) {
        list.hardFlipSoftSelected();
        softselecting = false;
        if (list.cursoredFile()->isHardSelected()) {
          list.cursoredFile()->unHardSelect();
        }
        else {
          list.cursoredFile()->hardSelect();
        }
        ui->redraw();
      }
      else if (list.cursoredFile() != NULL) {
        if (list.cursoredFile()->isHardSelected()) {
          list.cursoredFile()->unHardSelect();
        }
        else {
          list.cursoredFile()->hardSelect();
        }
        ui->update();
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
    case '-':
      if (list.cursoredFile() == NULL) {
        break;
      }
      temphighlightline = table.getElement(table.getSelectionPointer())->getRow();
      ui->redraw();
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
  return "[Esc] Cancel - [c]lose - [Up/Down] Navigate - [Enter/Right] open dir - [Backspace/Left] return - sp[r]ead - [v]iew file - [D]ownload - [b]ind to section - [s]ort - ra[w] command - [W]ipe - [Del]ete - [n]uke - [m]ake directory - Toggle se[P]arators - [q]uick jump - Toggle [f]ilter - view cwd [l]og - [Space] Hard select - [Shift-Up/Down] Soft select - Select [A]ll";
}

std::string BrowseScreenSite::getInfoLabel() const {
  return "BROWSING: " + site->getName();
}

std::string BrowseScreenSite::getInfoText() const {
  std::string text = list.getPath().toString();
  if (!requestids.empty()) {
    std::string target;
    int total = actionfiles.size();
    if (total == 1) {
      target = actionfiles.front().first;
    }
    else {
      int remaining = total - requestids.size();
      if (remaining <= 0) {
        remaining = 1;
      }
      target = "items (" + util::int2Str(remaining) + "/" + util::int2Str(total) + ")";
    }
    if (wipe) {
      text = "Wiping " + target + "  ";
    }
    else if (deleting) {
      text = "Deleting " + target + "  ";
    }
    else if (nuking) {
      text = "Nuking " + target + "  ";
    }
    else if (mkdiring) {
      text = "Making directory " + target + "  ";
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
  std::string target;
  int total = actionfiles.size();
  if (total == 1) {
    target = actionfiles.front().first;
  }
  else {
    target = util::int2Str(static_cast<int>(total)) + " items";
  }
  if (wipesuccess) {
    if (tickcount++ < 8) {
      text = "Wipe successful: " + target;
      return text;
    }
    else {
      wipesuccess = false;
    }
  }
  else if (wipefailed) {
    if (tickcount++ < 8) {
      text = "Wipe failed: " + target;
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
      text = "Delete successful: " + target;
      return text;
    }
    else {
      deletesuccess = false;
    }
  }
  else if (deletefailed) {
    if (tickcount++ < 8) {
      text = "Delete failed: " + target;
      return text;
    }
    else {
      deletefailed = false;
    }
  }
  else if (nukesuccess) {
    if (tickcount++ < 8) {
      text = "Nuke successful: " + target;
      return text;
    }
    else {
      nukesuccess = false;
    }
  }
  else if (nukefailed) {
    if (tickcount++ < 8) {
      text = "Nuke failed: " + target;
      return text;
    }
    else {
      nukefailed = false;
    }
  }
  else if (mkdirsuccess) {
    if (tickcount++ < 8) {
      text = "Directory created: " + target;
      return text;
    }
    else {
      mkdirsuccess = false;
    }
  }
  else if (mkdirfailed) {
    if (tickcount++ < 8) {
      text = "Directory creation failed: " + target;
      return text;
    }
    else {
      mkdirfailed = false;
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
  requestids.push_back(sitelogic->requestFileList(requestedpath));
}

void BrowseScreenSite::tick(int) {
  if (gotomode && !gotomodefirst) {
    if (gotomodeticker++ >= 20) {
      disableGotoMode();
    }
  }
}

void BrowseScreenSite::addFileDetails(MenuSelectOption & table, unsigned int coloffset, unsigned int y, const std::string & name) {
  addFileDetails(table, coloffset, y, "", name, "", "", "", false, false, NULL);
}

void BrowseScreenSite::addFileDetails(MenuSelectOption & table, unsigned int coloffset, unsigned int y, const std::string & prepchar, const std::string & name, const std::string & size, const std::string & lastmodified, const std::string & owner, bool selectable, bool cursored, UIFile * origin) {
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
  msotb->setOrigin(origin);
  if (cursored) {
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

void BrowseScreenSite::clearSoftSelects() {
  if (softselecting) {
    list.clearSoftSelected();
    ui->redraw();
    softselecting = false;
  }
}

UIFileList * BrowseScreenSite::getUIFileList() {
  return &list;
}

void BrowseScreenSite::printFlipped(const Pointer<ResizableElement> & re) {
  printFlipped(ui, re);
}

void BrowseScreenSite::printFlipped(Ui * ui, const Pointer<ResizableElement> & re) {
  int flipper = 0;
  for (unsigned int i = 0; i < re->getLabelText().length(); i++) {
    ui->printChar(re->getRow(), re->getCol() + i, re->getLabelText()[i], flipper++ % 2);
  }
}
