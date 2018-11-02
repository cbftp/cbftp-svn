#include "browsescreenlocal.h"



#include "../../core/tickpoke.h"
#include "../../localstorage.h"
#include "../../localfilelist.h"
#include "../../globalcontext.h"
#include "../../skiplist.h"
#include "../../util.h"

#include "../ui.h"
#include "../menuselectoptiontextbutton.h"
#include "../resizableelement.h"
#include "../termint.h"
#include "../menuselectadjustableline.h"
#include "../misc.h"

#include "browsescreenaction.h"

#include <cassert>

BrowseScreenLocal::BrowseScreenLocal(Ui * ui) : ui(ui), currentviewspan(0),
    focus(true), spinnerpos(0), tickcount(0),
    resort(false), sortmethod(UIFileList::SortMethod::COMBINED), gotomode(false), gotomodefirst(false),
    gotomodeticker(0), filtermodeinput(false), temphighlightline(-1),
    softselecting(false), lastinfo(LastInfo::NONE)
{
  BrowseScreenRequest request;
  request.type = BrowseScreenRequestType::FILELIST;
  request.path = global->getLocalStorage()->getDownloadPath();
  request.id = global->getLocalStorage()->requestLocalFileList(request.path);
  requests.push_back(request);
}

BrowseScreenLocal::~BrowseScreenLocal() {

}

BrowseScreenType BrowseScreenLocal::type() const {
  return BROWSESCREEN_LOCAL;
}

void BrowseScreenLocal::redraw(unsigned int row, unsigned int col, unsigned int coloffset) {
  row = row - (filtermodeinput ? 2 : 0);
  this->row = row;
  this->col = col;
  this->coloffset = coloffset;

  if (resort == true) {
    list.sortMethod(sortmethod);
    resort = false;
  }
  unsigned int position = list.currentCursorPosition();
  unsigned int listsize = list.size();
  adaptViewSpan(currentviewspan, row, position, listsize);

  table.reset();
  const std::vector<UIFile *> * uilist = list.getSortedList();
  for (unsigned int i = 0; i + currentviewspan < uilist->size() && i < row; i++) {
    unsigned int listi = i + currentviewspan;
    UIFile * uifile = (*uilist)[listi];
    bool selected = uifile == list.cursoredFile();
    bool isdir = uifile->isDirectory();
    bool allowed = global->getSkipList()->check((list.getPath() / uifile->getName()).toString(), isdir, false).action != SKIPLIST_DENY;
    std::string prepchar = " ";
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
    addFileDetails(table, coloffset, i, uifile->getName(), prepchar, uifile->getSizeRepr(), uifile->getLastModified(), owner, true, selected, uifile);
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

void BrowseScreenLocal::update() {
  if (handleReadyRequests()) {
    return;
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
    std::string pretag = "[Filter]: ";
    ui->printStr(filterfield.getRow(), coloffset + filterfield.getCol(), pretag + filterfield.getContentText());
    ui->moveCursor(filterfield.getRow(), coloffset + filterfield.getCol() + pretag.length() + filterfield.cursorPosition());
  }
}

bool BrowseScreenLocal::handleReadyRequests() {
  bool handled = false;
  while (!requests.empty() && global->getLocalStorage()->requestReady(requests.front().id)) {
    handled = true;
    const BrowseScreenRequest & request = requests.front();
    switch (request.type) {
      case BrowseScreenRequestType::PATH_INFO: {
        LocalPathInfo pathinfo = global->getLocalStorage()->getPathInfo(request.id);
        std::string dirs = std::to_string(pathinfo.getNumDirs());
        std::string files = std::to_string(pathinfo.getNumFiles());
        std::string size = util::parseSize(pathinfo.getSize());
        int maxdepth = pathinfo.getDepth();
        std::string targettext = "these " + util::int2Str(static_cast<int>(request.files.size())) + " items";
        if (request.files.size() == 1) {
          targettext = request.files.front().first;
        }
        std::string confirmation = "Do you really want to delete " + targettext + "?";
        if (maxdepth > 0) {
          std::string contain = request.files.size() == 1 ? " It contains " : " They contain ";
          std::string depthpre = request.files.size() == 1 ? "is " : "are ";
          confirmation += contain + size + " in " + files + "f/" + dirs + "d and " + depthpre;
        }
        if (maxdepth == 1) {
          confirmation += "1 level deep.";
        }
        else if (maxdepth > 1) {
          confirmation += util::int2Str(maxdepth) + " levels deep.";
        }
        if (pathinfo.getNumDirs() >= 10 || pathinfo.getNumFiles() >= 100 || pathinfo.getSize() >= 100000000000 || pathinfo.getDepth() > 2) {
          ui->goStrongConfirmation(confirmation);
        }
        else {
          ui->goConfirmation(confirmation);
        }
        break;
      }
      case BrowseScreenRequestType::FILELIST:
        gotoPath(request.id, request.path);
        requests.pop_front();
        break;
      case BrowseScreenRequestType::DELETE: {
        bool success = global->getLocalStorage()->getDeleteResult(request.id);
        if (success) {
          lastinfo = LastInfo::DELETE_SUCCESS;
        }
        else {
          lastinfo = LastInfo::DELETE_FAILED;
        }
        lastinfotarget = request.files.front().first;
        tickcount = 0;
        refreshFilelist();
        requests.pop_front();
        break;
      }
      default:
        assert(false);
        break;
    }
  }
  if (handled) {
    ui->redraw();
    ui->setInfo();
  }
  return handled;
}

void BrowseScreenLocal::command(const std::string & command, const std::string & arg) {
  if (!requests.empty() && requests.front().type == BrowseScreenRequestType::PATH_INFO) {
    const BrowseScreenRequest & pathinforequest = requests.front();
    if (command == "yes") {
      for (const auto & file : pathinforequest.files) {
        BrowseScreenRequest delrequest;
        delrequest.path = pathinforequest.path;
        std::list<std::pair<std::string, bool>> files;
        files.push_back(file);
        delrequest.files = files;
        delrequest.type = BrowseScreenRequestType::DELETE;
        delrequest.id = global->getLocalStorage()->requestDelete(delrequest.path / file.first, true);
        requests.push_back(delrequest);
      }
    }
    requests.pop_front();
    ui->setInfo();
  }
  ui->redraw();
}

BrowseScreenAction BrowseScreenLocal::keyPressed(unsigned int ch) {
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
    case 'W':
    case KEY_DC: {
      std::list<std::pair<std::string, bool> > filenames = list.getSelectedNames();
      if (filenames.empty()) {
        break;
      }
      BrowseScreenRequest request;
      request.path = filelist->getPath();
      request.files = filenames;
      request.type = BrowseScreenRequestType::PATH_INFO;
      std::list<Path> paths;
      for (std::list<std::pair<std::string, bool> >::const_iterator it = filenames.begin(); it != filenames.end(); it++) {
        paths.emplace_back(request.path / it->first);
      }
      request.id = global->getLocalStorage()->requestPathInfo(paths);
      requests.push_back(request);
      ui->setInfo();
      break;
    }
    case 'q':
      gotomode = true;
      gotomodefirst = true;
      gotomodeticker = 0;
      gotomodestring = "";
      global->getTickPoke()->startPoke(this, "BrowseScreenLocal", 50, 0);
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
    {
      //go up one directory level, or return if at top already
      const Path & oldpath = list.getPath();
      if (oldpath == "/") {
        return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
      }
      BrowseScreenRequest request;
      request.type = BrowseScreenRequestType::FILELIST;
      request.path = oldpath.dirName();
      request.id = global->getLocalStorage()->requestLocalFileList(request.path);
      requests.push_back(request);
      ui->setInfo();
      ui->redraw();
      break;
    }
    case KEY_F(5):
    {
      refreshFilelist();
      ui->setInfo();
      break;
    }
    case KEY_DOWN:
      //go down and highlight next item (if not at bottom already)
      clearSoftSelects();
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
      clearSoftSelects();
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
    case KEY_RIGHT:
    case 10:
    {
      UIFile * cursoredfile = list.cursoredFile();
      if (cursoredfile == NULL) {
        break;
      }
      bool isdir = cursoredfile->isDirectory();
      bool islink = cursoredfile->isLink();
      if (isdir || islink) {
        BrowseScreenRequest request;
        request.type = BrowseScreenRequestType::FILELIST;
        const Path & oldpath = list.getPath();
        if (islink) {
          Path target = cursoredfile->getLinkTarget();
          if (target.isAbsolute()) {
            request.path = target;
          }
          else {
            request.path = oldpath / target;
          }
        }
        else {
          request.path = oldpath / cursoredfile->getName();
        }
        request.id = global->getLocalStorage()->requestLocalFileList(request.path);
        requests.push_back(request);
        ui->setInfo();
        ui->redraw();
      }
      break;
    }
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
    case 'v':
      //view selected file, do nothing if a directory is selected
      if (list.cursoredFile() != NULL && !list.cursoredFile()->isDirectory()) {
        ui->goViewFile(filelist->getPath(), list.cursoredFile()->getName());
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

std::string BrowseScreenLocal::getLegendText() const {
  if (gotomode) {
    return "[Any] Go to first matching entry name - [Esc] Cancel";
  }
  if (filtermodeinput) {
    return "[Any] Enter space separated filters. Valid operators are !, *, ?. Must match all negative filters and at least one positive if given. Case insensitive. - [Esc] Cancel";
  }
  return "[Up/Down] Navigate - [Enter/Right] open dir - [s]ort - [Backspace/Left] return - [Esc] Cancel - [c]lose - [q]uick jump - Toggle [f]ilter - [Space] Hard select - [Shift-Up/Down] Soft select - Select [A]ll";
}

std::string BrowseScreenLocal::getInfoLabel() const {
  return "LOCAL BROWSING";
}

std::string BrowseScreenLocal::getInfoText() const {
  std::string text = list.getPath().toString();
  if (!requests.empty()) {
    const BrowseScreenRequest & request = requests.front();
    std::string target = targetName(request.files);
    if (requests.size() > 1) {
      target += " (+ " + std::to_string(requests.size()) + " more)";
    }
    switch (request.type) {
      case BrowseScreenRequestType::DELETE:
        text = "Deleting " + target + "  ";
        break;
      case BrowseScreenRequestType::PATH_INFO:
        text = "Summarizing " + target + "  ";
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
        text = "CWD failed: ";
        break;
      case LastInfo::DELETE_SUCCESS:
        text = "Delete successful: ";
        break;
      case LastInfo::DELETE_FAILED:
        text = "Delete failed: ";
        break;
      case LastInfo::NONE:
      default:
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

void BrowseScreenLocal::setFocus(bool focus) {
  this->focus = focus;
  if (!focus) {
    disableGotoMode();
    filtermodeinput = false;
  }
}

void BrowseScreenLocal::refreshFilelist() {
  BrowseScreenRequest request;
  request.type = BrowseScreenRequestType::FILELIST;
  request.path = list.getPath();
  request.id = global->getLocalStorage()->requestLocalFileList(request.path);
  requests.push_back(request);
}

void BrowseScreenLocal::tick(int) {
  if (gotomode && !gotomodefirst) {
    if (gotomodeticker++ >= 20) {
      disableGotoMode();
    }
  }
}

UIFile * BrowseScreenLocal::selectedFile() const {
  return list.cursoredFile();
}

void BrowseScreenLocal::disableGotoMode() {
  if (gotomode) {
    gotomode = false;
    global->getTickPoke()->stopPoke(this, 0);
    ui->setLegend();
  }
}

void BrowseScreenLocal::gotoPath(int requestid, const Path & path) {
  std::shared_ptr<LocalFileList> filelist = global->getLocalStorage()->getLocalFileList(requestid);
  if (!filelist) {
    lastinfo = LastInfo::CWD_FAILED;
    lastinfotarget = path.toString();
    tickcount = 0;
    return;
  }
  this->filelist = filelist;
  if (list.cursoredFile() != NULL) {
    selectionhistory.push_front(std::pair<Path, std::string>(list.getPath(), list.cursoredFile()->getName()));
  }
  else {
    currentviewspan = 0;
  }
  unsigned int position = 0;
  std::list<std::string> filters;
  std::set<std::string> uniques;
  if (list.getPath() == filelist->getPath()) {
    position = list.currentCursorPosition();
    filters = list.getFilters();
    uniques = list.getUniques();
  }
  list.parse(filelist);
  if (filters.size()) {
    list.setFilters(filters);
  }
  list.sortMethod(sortmethod);
  for (std::list<std::pair<Path, std::string> >::iterator it = selectionhistory.begin(); it != selectionhistory.end(); it++) {
    if (it->first == filelist->getPath()) {
      bool selected = list.selectFileName(it->second);
      selectionhistory.erase(it);
      if (selected) {
        return;
      }
      break;
    }
  }
  if (position) {
    list.setCursorPosition(position);
  }
}

std::shared_ptr<LocalFileList> BrowseScreenLocal::fileList() const {
  return filelist;
}

void BrowseScreenLocal::clearSoftSelects() {
  if (softselecting) {
    list.clearSoftSelected();
    ui->redraw();
    softselecting = false;
  }
}

UIFileList * BrowseScreenLocal::getUIFileList() {
  return &list;
}
