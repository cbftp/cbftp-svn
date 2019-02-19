#include "browsescreensite.h"

#include <algorithm>
#include <cctype>
#include <memory>

#include "../../core/tickpoke.h"
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
#include "../../sectionmanager.h"
#include "../../section.h"

#include <cassert>

#include "../ui.h"
#include "../uifile.h"
#include "../termint.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectadjustableline.h"
#include "../resizableelement.h"
#include "../misc.h"

#include "browsescreenaction.h"
#include "rawdatascreen.h"

BrowseScreenSite::BrowseScreenSite(Ui * ui, const std::string & sitestr, const Path path) :
    ui(ui), row(0), col(0), coloffset(0), currentviewspan(0),
    resort(false), tickcount(0), gotomode(false), gotomodefirst(false),
    gotomodeticker(0), filtermodeinput(false),
    sortmethod(UIFileList::SortMethod::COMBINED),
    sitelogic(global->getSiteLogicManager()->getSiteLogic(sitestr)),
    site(sitelogic->getSite()), spinnerpos(0), filelist(nullptr),
    withinraceskiplistreach(false), focus(true), temphighlightline(-1),
    softselecting(false), lastinfo(LastInfo::NONE),
    confirmaction(ConfirmAction::NONE), refreshfilelistafter(false)
{
  sitelogic->getAggregatedRawBuffer()->bookmark();
  BrowseScreenRequest request;
  request.type = BrowseScreenRequestType::FILELIST;
  request.path = path != "" ? path : site->getBasePath();
  request.id = sitelogic->requestFileList(request.path);
  requests.push_back(request);
  TimeReference::updateTime();
}

BrowseScreenSite::~BrowseScreenSite() {
  disableGotoMode();
  if (!list.isInitialized()) {
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
  if (!list.isInitialized()) {
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
    list.sortMethod(sortmethod);
    resort = false;
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
    SkipList * sectionskiplist = nullptr;
    Section * section = global->getSectionManager()->getSection(closestracesection);
    if (section) {
      sectionskiplist = &section->getSkipList();
    }
    SkipListMatch match = site->getSkipList().check((prepend / uifile->getName()).toString(), isdir, withinraceskiplistreach, sectionskiplist);
    if (match.action == SKIPLIST_SIMILAR) {
      if (!filelist->similarChecked()) {
        filelist->checkSimilar(site->getSkipList().getSimilarPatterns(sectionskiplist));
      }
    }
    bool allowed = !(match.action == SKIPLIST_DENY ||
                    (match.action == SKIPLIST_UNIQUE &&
                     filelist->containsPatternBefore(match.matchpattern, isdir, uifile->getName())) ||
                    (match.action == SKIPLIST_SIMILAR &&
                     filelist->containsUnsimilar(uifile->getName())));
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
    addFileDetails(table, coloffset, i, uifile->getName(), prepchar, uifile->getSizeRepr(),
        uifile->getLastModified(), owner, true, cursored, uifile);
  }
  table.adjustLines(col - 3);
  table.checkPointer();
  if (temphighlightline != -1) {
    std::shared_ptr<MenuSelectAdjustableLine> highlightline = table.getAdjustableLineOnRow(temphighlightline);
    if (!!highlightline) {
      std::pair<unsigned int, unsigned int> minmaxcol = highlightline->getMinMaxCol();
      for (unsigned int i = minmaxcol.first; i <= minmaxcol.second; i++) {
        ui->printChar(temphighlightline, i, ' ', true);
      }
    }
  }
  for (unsigned int i = 0; i < table.size(); i++) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(i));
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
        printFlipped(ui, re);
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
  if (!list.isInitialized()) {
    ui->redraw();
  }
  if (table.size()) {
    std::shared_ptr<ResizableElement> re = std::static_pointer_cast<ResizableElement>(table.getElement(table.getLastSelectionPointer()));
    bool softselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isSoftSelected();
    bool hardselected = re->getOrigin() && static_cast<UIFile *>(re->getOrigin())->isHardSelected();
    if (softselected && hardselected) {
      printFlipped(ui, re);
    }
    else {
      ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), softselected || hardselected);
    }
    re = std::static_pointer_cast<ResizableElement>(table.getElement(table.getSelectionPointer()));
    bool selected = re->getOrigin() && (static_cast<UIFile *>(re->getOrigin())->isSoftSelected() ||
                                        static_cast<UIFile *>(re->getOrigin())->isHardSelected());
    if (selected && focus) {
      printFlipped(ui, re);
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
  bool handled = false;
  while (!requests.empty() && sitelogic->requestReady(requests.front().id)) {
    handled = true;
    const BrowseScreenRequest & request = requests.front();
    switch (request.type) {
      case BrowseScreenRequestType::FILELIST:
        loadFileListFromRequest();
        break;
      case BrowseScreenRequestType::NUKE: {
        bool success = sitelogic->finishRequest(request.id);
        if (success) {
          lastinfo = LastInfo::NUKE_SUCCESS;
          if (list.getPath() == request.path) {
            refreshfilelistafter = true;
          }
        }
        else {
          lastinfo = LastInfo::NUKE_FAILED;
        }
        lastinfotarget = request.files.front().first;
        tickcount = 0;
        break;
      }
      case BrowseScreenRequestType::WIPE: {
        bool success = sitelogic->finishRequest(request.id);
        if (success) {
          lastinfo = LastInfo::WIPE_SUCCESS;
          if (list.getPath() == request.path) {
            refreshfilelistafter = true;
          }
        }
        else {
          lastinfo = LastInfo::WIPE_FAILED;
        }
        lastinfotarget = request.files.front().first;
        tickcount = 0;
        break;
      }
      case BrowseScreenRequestType::MKDIR: {
        bool success = sitelogic->finishRequest(request.id);
        if (success) {
          lastinfo = LastInfo::MKDIR_SUCCESS;
          if (list.getPath() == request.path) {
            refreshfilelistafter = true;
          }
        }
        else {
          lastinfo = LastInfo::MKDIR_FAILED;
        }
        lastinfotarget = request.files.front().first;
        tickcount = 0;
        break;
      }
      case BrowseScreenRequestType::DELETE: {
        bool success = sitelogic->finishRequest(request.id);
        if (success) {
          lastinfo = LastInfo::DELETE_SUCCESS;
          if (list.getPath() == request.path) {
            refreshfilelistafter = true;
          }
        }
        else {
          lastinfo = LastInfo::DELETE_FAILED;
        }
        lastinfotarget = request.files.front().first;
        tickcount = 0;
        break;
      }
      default:
        assert(false);
        break;
    }
    requests.pop_front();
  }
  if (handled) {
    if (requests.empty() && refreshfilelistafter) {
      refreshfilelistafter = false;
      refreshFilelist();
    }
    ui->redraw();
    ui->setInfo();
  }
  return handled;
}

void BrowseScreenSite::loadFileListFromRequest() {
  assert(!requests.empty());
  const BrowseScreenRequest & request = requests.front();
  FileListData * newfilelistdata = sitelogic->getFileListData(request.id);
  if (newfilelistdata == nullptr) {
    sitelogic->finishRequest(request.id);
    lastinfo = LastInfo::CWD_FAILED;
    tickcount = 0;
    return;
  }
  cwdrawbuffer = newfilelistdata->getCwdRawBuffer();
  filelist = newfilelistdata->getFileList();
  if (list.isInitialized()) {
    if (list.cursoredFile() != nullptr) {
      selectionhistory.push_front(std::pair<Path, std::string>(list.getPath(), list.cursoredFile()->getName()));
    }
  }
  else {
    sitelogic->getAggregatedRawBuffer()->setUiWatching(false);
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
    lastinfo = LastInfo::NONE;
  }
  list.parse(filelist);
  sitelogic->finishRequest(request.id);
  if (separatorsenabled) {
    list.toggleSeparators();
  }
  if (filters.size()) {
    list.setFilters(filters);
  }
  if (uniques.size()) {
    list.setUnique(uniques);
  }
  list.sortMethod(sortmethod);
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
        closestracesection = *it;
      }
    }
    else {
      withinraceskiplistreach = false;
      break;
    }
  }
  //delete filelist;
}

void BrowseScreenSite::command(const std::string & command, const std::string & arg) {
  if (command == "yes") {
    switch (confirmaction) {
      case ConfirmAction::WIPE:
        for (const std::pair<std::string, bool> & file : list.getSelectedNames()) {
          BrowseScreenRequest request;
          request.id = sitelogic->requestWipe(list.getPath() / file.first, file.second);
          request.type = BrowseScreenRequestType::WIPE;
          request.path = list.getPath();
          request.files = { file };
          requests.push_back(request);
        }
        break;
      case ConfirmAction::DELETE:
        for (const std::pair<std::string, bool> & file : list.getSelectedNames()) {
          BrowseScreenRequest request;
          request.id = sitelogic->requestDelete(list.getPath() / file.first, file.second, true, true);
          request.type = BrowseScreenRequestType::DELETE;
          request.path = list.getPath();
          request.files = { file };
          requests.push_back(request);
        }
        break;
      default:
        assert(false);
        break;
    }
    confirmaction = ConfirmAction::NONE;
    ui->redraw();
    ui->setInfo();
  }
  else if (command == "no") {
    confirmaction = ConfirmAction::NONE;
    ui->redraw();
  }
  else if (command == "makedir") {
    BrowseScreenRequest request;
    request.id = sitelogic->requestMakeDirectory(arg);
    request.type = BrowseScreenRequestType::MKDIR;
    request.path = list.getPath();
    request.files = { std::pair<std::string, bool>(arg, true) };
    requests.push_back(request);
    ui->redraw();
    ui->setInfo();
  }
  else if (command == "nuke") {
    std::vector<std::string> args = util::splitVec(arg, ";");
    int multiplier = std::stoi(args[0]);
    std::string reason = args[1];
    for (const std::pair<std::string, bool> & item : list.getSelectedDirectoryNames()) {
      BrowseScreenRequest request;
      request.id = sitelogic->requestNuke(list.getPath() / item.first, multiplier, reason);
      request.type = BrowseScreenRequestType::NUKE;
      request.path = list.getPath();
      request.files = { item };
      requests.push_back(request);
    }
    ui->redraw();
    ui->setInfo();
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
        list.setFilters(util::trim(util::split(filter)));
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
        ui->goInfo("Spread jobs are only applicable for directories. Please select one or more directories to start a spread job.");
        break;
      }
      std::list<std::string> sections = site->getSectionsForPath(list.getPath());
      if (sections.empty()) {
        ui->goInfo("Cannot start a spread job here since no section is bound to this directory. Please bind a section to this directory first.");
        break;
      }
      ui->goNewRace(site->getName(), sections, items);
      break;
    }
    case 'b':
      ui->goAddSiteSection(site, list.getPath());
      break;
    case 'v':
      viewCursored();
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
      sortmethod = static_cast<UIFileList::SortMethod>((static_cast<int>(sortmethod) + 1) % 9);
      resort = true;
      lastinfo = LastInfo::CHANGED_SORT;
      lastinfotarget = list.getSortMethod(sortmethod);
      tickcount = 0;
      ui->redraw();
      ui->setInfo();
      break;
    case 'S':
      sortmethod = UIFileList::SortMethod::COMBINED;
      resort = true;
      lastinfo = LastInfo::CHANGED_SORT;
      lastinfotarget = list.getSortMethod(sortmethod);
      tickcount = 0;
      ui->redraw();
      ui->setInfo();
      break;
    case 'P':
      resort = true;
      list.toggleSeparators();
      ui->redraw();
      break;
    case 'n': {
      std::list<std::pair<std::string, bool>> dirs = list.getSelectedDirectoryNames();
      if (dirs.empty()) {
        ui->goInfo("Please select directories for nuking.");
        break;
      }
      ui->goNuke(site->getName(), targetName(dirs), list.getPath());
      break;
    }
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
        Path requestedpath;
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
        BrowseScreenRequest request;
        request.id = sitelogic->requestFileList(requestedpath);
        request.type = BrowseScreenRequestType::FILELIST;
        request.path = requestedpath;
        requests.push_back(request);
        ui->update();
        ui->setInfo();
      }
      else {
        viewCursored();
      }
      return BrowseScreenAction(BROWSESCREENACTION_CHDIR);
    case 'W': {
      std::list<std::pair<std::string, bool> > filenames = list.getSelectedNames();
      if (filenames.empty()) {
        break;
      }
      confirmaction = ConfirmAction::WIPE;
      std::string targettext = "these " + std::to_string(static_cast<int>(filenames.size())) + " items";
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
      confirmaction = ConfirmAction::DELETE;
      std::string targettext = "these " + std::to_string(static_cast<int>(filenames.size())) + " items";
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
    case KEY_BACKSPACE: {
      oldpath = list.getPath();
      if (oldpath == "/") {
        return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
      }
      Path requestedpath = oldpath.cutLevels(1);
      BrowseScreenRequest request;
      request.id = sitelogic->requestFileList(requestedpath);
      request.type = BrowseScreenRequestType::FILELIST;
      request.path = requestedpath;
      requests.push_back(request);
      ui->setInfo();
      //go up one directory level, or return if at top already
      return BrowseScreenAction(BROWSESCREENACTION_CHDIR);
    }
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
  if (!requests.empty()) {
    const BrowseScreenRequest & request = requests.front();
    std::string target = targetName(request.files);
    if (requests.size() > 1) {
      target += " (+" + std::to_string(requests.size() - 1) + " more)";
    }
    switch (request.type) {
      case BrowseScreenRequestType::WIPE:
        text = "Wiping " + target + "  ";
        break;
      case BrowseScreenRequestType::DELETE:
        text = "Deleting " + target + "  ";
        break;
      case BrowseScreenRequestType::NUKE:
        text = "Nuking " + target + "  ";
        break;
      case BrowseScreenRequestType::MKDIR:
        text = "Making directory " + target + "  ";
        break;
      case BrowseScreenRequestType::FILELIST:
        text = "Getting list for " + request.path.toString() + "  ";
        break;
      default:
        assert(false);
        break;
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
  if (tickcount++ < 8 && lastinfo != LastInfo::NONE) {
    switch (lastinfo) {
      case LastInfo::CHANGED_SORT:
        text = "Sort method: ";
        break;
      case LastInfo::CWD_FAILED:
        text = "Getting file list failed: ";
        break;
      case LastInfo::DELETE_SUCCESS:
        text = "Delete successful: ";
        break;
      case LastInfo::DELETE_FAILED:
        text = "Delete failed: ";
        break;
      case LastInfo::WIPE_SUCCESS:
        text = "Wipe successful: ";
        break;
      case LastInfo::WIPE_FAILED:
        text = "Wipe failed: ";
        break;
      case LastInfo::MKDIR_SUCCESS:
        text = "Directory created: ";
        break;
      case LastInfo::MKDIR_FAILED:
        text = "Directory creation failed: ";
        break;
      case LastInfo::NUKE_SUCCESS:
        text = "Nuke successful: ";
        break;
      case LastInfo::NUKE_FAILED:
        text = "Nuke failed: ";
        break;
      case LastInfo::NONE:
        break;
    }
    return text + lastinfotarget;
  }
  if (list.hasFilters() || list.hasUnique()) {
    if (list.hasFilters()) {
      text += std::string("  FILTER: ") + filterfield.getData();
    }
    if (list.hasUnique()) {
      text += "  UNIQUES";
    }
    text += "  " + std::to_string(list.filteredSizeFiles()) + "/" + std::to_string(list.sizeFiles()) + "f " +
        std::to_string(list.filteredSizeDirs()) + "/" + std::to_string(list.sizeDirs()) + "d";
    text += std::string("  ") + util::parseSize(list.getFilteredTotalSize()) + "/" +
        util::parseSize(list.getTotalSize());
  }
  else {
    text += "  " + std::to_string(list.sizeFiles()) + "f " + std::to_string(list.sizeDirs()) + "d";
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

void BrowseScreenSite::refreshFilelist() {
  BrowseScreenRequest request;
  request.id = sitelogic->requestFileList(list.getPath());
  request.type = BrowseScreenRequestType::FILELIST;
  request.path = list.getPath();
  requests.push_back(request);
}

void BrowseScreenSite::tick(int) {
  if (gotomode && !gotomodefirst) {
    if (gotomodeticker++ >= 20) {
      disableGotoMode();
    }
  }
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

const std::shared_ptr<Site> & BrowseScreenSite::getSite() const {
  return site;
}

void BrowseScreenSite::viewCursored() {
  //view selected file, do nothing if a directory is selected
  if (list.cursoredFile() != NULL) {
    if (list.cursoredFile()->isDirectory()) {
      ui->goInfo("Cannot use the file viewer on a directory.");
    }
    else {
      ui->goViewFile(site->getName(), list.cursoredFile()->getName(), filelist);
    }
  }
}
