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

BrowseScreenLocal::BrowseScreenLocal(Ui * ui) : ui(ui), currentviewspan(0),
    focus(true), changedsort(false), cwdfailed(false), tickcount(0),
    resort(false), sortmethod(0), gotomode(false), gotomodefirst(false),
    gotomodeticker(0), filtermodeinput(false)
{
  gotoPath(global->getLocalStorage()->getDownloadPath());
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
    sort();
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
    bool allowed = global->getSkipList()->isAllowed(list.getPath() + "/" + uifile->getName(), isdir, false);
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
    addFileDetails(i, prepchar, uifile->getName(), uifile->getSizeRepr(), uifile->getLastModified(), owner, true, selected);
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
    filterfield = MenuSelectOptionTextField("filter", row + 1, 1, "", oldtext, col - 15, col - 15, false);
    ui->showCursor();
  }
  update();
}

void BrowseScreenLocal::update() {
  if (table.size()) {
    Pointer<ResizableElement> re = table.getElement(table.getLastSelectionPointer());
    ui->printStr(re->getRow(), re->getCol(), re->getLabelText());
    re = table.getElement(table.getSelectionPointer());
    ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), focus);
  }
  if (filtermodeinput) {
    std::string pretag = "[Filter]: ";
    ui->printStr(filterfield.getRow(), coloffset + filterfield.getCol(), pretag + filterfield.getContentText());
    ui->moveCursor(filterfield.getRow(), coloffset + filterfield.getCol() + pretag.length() + filterfield.cursorPosition());
  }
}

void BrowseScreenLocal::command(std::string, std::string) {

}

BrowseScreenAction BrowseScreenLocal::keyPressed(unsigned int ch) {
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
        list.setFilter(filter);
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
      if (list.hasFilter()) {
        resort = true;
        list.unsetFilter();
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
      std::string oldpath = list.getPath();
      if (oldpath == "/" ) {
        return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
      }
      size_t position = oldpath.rfind("/");
      if (position == 0) {
        position = 1;
      }
      std::string targetpath = oldpath.substr(0, position);
      gotoPath(targetpath);
      ui->setInfo();
      ui->redraw();
      break;
    }
    case KEY_F(5):
    {
      gotoPath(list.getPath());
      ui->setInfo();
      ui->redraw();
      break;
    }
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
        std::string oldpath = list.getPath();
        if (oldpath.length() > 1) {
          oldpath += "/";
        }
        std::string targetpath;
        if (islink) {
          std::string target = cursoredfile->getLinkTarget();
          if (target.length() > 0 && target[0] == '/') {
            targetpath = target;
          }
          else {
            targetpath = oldpath + target;
          }
        }
        else {
          targetpath = oldpath + cursoredfile->getName();
        }
        gotoPath(targetpath);
        ui->setInfo();
        ui->redraw();
      }
      break;
    }
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
    case 'v':
      //view selected file, do nothing if a directory is selected
      if (list.cursoredFile() != NULL && !list.cursoredFile()->isDirectory()) {
        ui->goViewFile(filelist->getPath(), list.cursoredFile()->getName());
      }
      break;
  }
  return BrowseScreenAction();
}

std::string BrowseScreenLocal::getLegendText() const {
  if (gotomode) {
    return "[Any] Go to first matching entry name - [Esc] Cancel";
  }
  if (filtermodeinput) {
    return "[Any] Enter filter text - [Esc] Cancel";
  }
  return "[Up/Down] Navigate - [Enter/Right] open dir - [s]ort - [Backspace/Left] return - [Esc] Cancel - [c]lose - [q]uick jump - Toggle [f]ilter";
}

std::string BrowseScreenLocal::getInfoLabel() const {
  return "LOCAL BROWSING";
}

std::string BrowseScreenLocal::getInfoText() const {
  std::string text = list.getPath();
  if (changedsort) {
    if (tickcount++ < 8) {
      text = "Sort method: " + list.getSortMethod();
      return text;
    }
    else {
      changedsort = false;
    }
  }
  else if (cwdfailed) {
    if (tickcount++ < 8) {
      text = "CWD failed: " + targetpath;
      return text;
    }
    else {
      cwdfailed = false;
    }
  }
  if (list.hasFilter()) {
    text += std::string("  FILTER: ") + filterfield.getData();
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

void BrowseScreenLocal::tick(int) {
  if (gotomode && !gotomodefirst) {
    if (gotomodeticker++ >= 20) {
      disableGotoMode();
    }
  }
}

void BrowseScreenLocal::addFileDetails(unsigned int y, std::string name) {
  addFileDetails(y, "", name, "", "", "", false, false);
}

void BrowseScreenLocal::addFileDetails(unsigned int y, std::string prepchar, std::string name, std::string size, std::string lastmodified, std::string owner, bool selectable, bool selected) {
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

UIFile * BrowseScreenLocal::selectedFile() const {
  return list.cursoredFile();
}

void BrowseScreenLocal::sort() {
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

void BrowseScreenLocal::disableGotoMode() {
  if (gotomode) {
    gotomode = false;
    global->getTickPoke()->stopPoke(this, 0);
    ui->setLegend();
  }
}

void BrowseScreenLocal::gotoPath(const std::string & path) {
  targetpath = path;
  Pointer<LocalFileList> filelist = global->getLocalStorage()->getLocalFileList(path);
  if (!filelist) {
    cwdfailed = true;
    tickcount = 0;
    return;
  }
  this->filelist = filelist;
  if (list.cursoredFile() != NULL) {
    selectionhistory.push_front(std::pair<std::string, std::string>(list.getPath(), list.cursoredFile()->getName()));
  }
  else {
    currentviewspan = 0;
  }
  bool setfilter = false;
  std::string filter;
  if (list.getPath() == filelist->getPath()) {
    setfilter = list.hasFilter();
    filter = list.getFilter();
  }
  list.parse(filelist);
  if (setfilter) {
    list.setFilter(filter);
  }
  sort();
  for (std::list<std::pair<std::string, std::string> >::iterator it = selectionhistory.begin(); it != selectionhistory.end(); it++) {
    if (it->first == path) {
      list.selectFileName(it->second);
      selectionhistory.erase(it);
      break;
    }
  }
}

Pointer<LocalFileList> BrowseScreenLocal::fileList() const {
  return filelist;
}
