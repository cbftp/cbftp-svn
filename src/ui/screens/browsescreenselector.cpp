#include "browsescreenselector.h"

#include "../../core/pointer.h"
#include "../../sitemanager.h"
#include "../../site.h"
#include "../../globalcontext.h"
#include "../../localstorage.h"

#include "../termint.h"
#include "../ui.h"
#include "../menuselectoptiontextbutton.h"
#include "../resizableelement.h"

#include "browsescreenaction.h"

BrowseScreenSelector::BrowseScreenSelector(Ui * ui) :
  ui(ui),
  focus(true),
  pointer(0),
  currentviewspan(0) {
  SiteManager * sm = global->getSiteManager();
  std::vector<Site *>::const_iterator it;
  entries.push_back(std::pair<std::string, std::string>(BROWSESCREENSELECTOR_HOME,
                                                        global->getLocalStorage()->getDownloadPath()));
  entries.push_back(std::pair<std::string, std::string>("", ""));
  for (it = sm->begin(); it != sm->end(); it++) {
    entries.push_back(std::pair<std::string, std::string>((*it)->getName(), (*it)->getName()));
  }
}

BrowseScreenSelector::~BrowseScreenSelector() {

}

BrowseScreenType BrowseScreenSelector::type() const {
  return BROWSESCREEN_SELECTOR;
}

void BrowseScreenSelector::redraw(unsigned int row, unsigned int col, unsigned int coloffset) {
  this->row = row;
  this->col = col;
  this->coloffset = coloffset;
  table.clear();
  unsigned int pagerows = (unsigned int) row / 2;
  if (pointer < currentviewspan || pointer >= currentviewspan + row) {
    if (pointer < pagerows) {
      currentviewspan = 0;
    }
    else {
      currentviewspan = pointer - pagerows;
    }
  }
  if (currentviewspan + row >= entries.size() && entries.size() + 1 >= row) {
    currentviewspan = entries.size() + 1 - row;
    if (currentviewspan > pointer) {
      currentviewspan = pointer;
    }
  }
  int y = 0;
  for (unsigned int i = currentviewspan; i < currentviewspan + row && i < entries.size(); i++) {
    Pointer<MenuSelectOptionTextButton> msotb =
        table.addTextButtonNoContent(y++, coloffset + 1, entries[i].first, entries[i].second);
    if (entries[i].first == "") {
      msotb->setSelectable(false);
    }
    if (i == pointer) {
      table.setPointer(msotb);
    }
  }
  table.checkPointer();
  for (unsigned int i = 0; i < table.size(); i++) {
    Pointer<MenuSelectOptionTextButton> msotb = table.getElement(i);
    bool highlight = false;
    if (table.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msotb->getRow(), msotb->getCol(), msotb->getLabelText(), highlight && focus);
  }
  unsigned int slidersize = 0;
  unsigned int sliderstart = 0;
  unsigned int listsize = entries.size();
  if (listsize > row) {
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
    for (unsigned int i = 0; i < row; i++) {
      if (i >= sliderstart && i < sliderstart + slidersize) {
        ui->printChar(i, coloffset + col - 1, ' ', true);
      }
      else {
        ui->printChar(i, coloffset + col - 1, BOX_VLINE);
      }
    }
  }
}

void BrowseScreenSelector::update() {
  if (table.size()) {
    if (pointer < currentviewspan || pointer >= currentviewspan + row) {
      ui->redraw();
      return;
    }
    Pointer<ResizableElement> re = table.getElement(table.getLastSelectionPointer());
    ui->printStr(re->getRow(), re->getCol(), re->getLabelText());
    re = table.getElement(table.getSelectionPointer());
    ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), focus);
  }
}

void BrowseScreenSelector::command(std::string, std::string) {

}

BrowseScreenAction BrowseScreenSelector::keyPressed(unsigned int ch) {
  unsigned int pagerows = (unsigned int) row * 0.6;
  switch (ch) {
    case 27: // esc
      ui->returnToLast();
      break;
    case 'c':
    case KEY_LEFT:
    case KEY_BACKSPACE:
      return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
    case KEY_UP:
      if (table.goUp()) {
        pointer = table.getElement(table.getSelectionPointer())->getRow() + currentviewspan;
        ui->update();
      }
      else if (pointer > 0) {
        pointer--;
        ui->redraw();
      }
      break;
    case KEY_DOWN:
      if (table.goDown()) {
        pointer = table.getElement(table.getSelectionPointer())->getRow() + currentviewspan;
        ui->update();
      }
      else if (pointer < entries.size()) {
        pointer++;
        ui->redraw();
      }
      break;
    case KEY_HOME:
      pointer = 0;
      ui->redraw();
      break;
    case KEY_END:
      pointer = entries.size() - 1;
      ui->redraw();
      break;
    case KEY_NPAGE:
      if (pagerows < entries.size() && pointer < entries.size() - 1 - pagerows) {
        pointer += pagerows;
      }
      else {
        pointer = entries.size() - 1;
      }
      ui->redraw();
      break;
    case KEY_PPAGE:
      if (pointer > pagerows) {
        pointer -= pagerows;
      }
      else {
        pointer = 0;
      }
      ui->redraw();
      break;
    case KEY_RIGHT:
    case 10:
    case 'b':
    {
      Pointer<MenuSelectOptionTextButton> msotb = table.getElement(table.getSelectionPointer());
      if (msotb->getIdentifier() == BROWSESCREENSELECTOR_HOME) {
        return BrowseScreenAction(BROWSESCREENACTION_HOME);
      }
      else {
        return BrowseScreenAction(BROWSESCREENACTION_SITE, msotb->getIdentifier());
      }
    }
      break;
  }
  return BrowseScreenAction();
}

std::string BrowseScreenSelector::getLegendText() const {
  return "[Up/Down] Navigate - [Enter/Right/b] Browse - [Esc] Cancel - [c]lose";
}

std::string BrowseScreenSelector::getInfoLabel() const {
  return "SELECT SITE";
}

std::string BrowseScreenSelector::getInfoText() const {
  return "select site or local";
}

void BrowseScreenSelector::setFocus(bool focus) {
  this->focus = focus;
}
