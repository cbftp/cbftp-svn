#include "browsescreenlocal.h"

#include "../ui.h"
#include "../menuselectoptiontextbutton.h"
#include "../resizableelement.h"

#include "browsescreenaction.h"

BrowseScreenLocal::BrowseScreenLocal(Ui * ui) : ui(ui), focus(true) {

}

BrowseScreenLocal::~BrowseScreenLocal() {

}

BrowseScreenType BrowseScreenLocal::type() const {
  return BROWSESCREEN_LOCAL;
}

void BrowseScreenLocal::redraw(unsigned int row, unsigned int col, unsigned int coloffset) {
  this->row = row;
  this->col = col;
  this->coloffset = coloffset;

  table.clear();

  table.checkPointer();
  for (unsigned int i = 0; i < table.size(); i++) {
    Pointer<ResizableElement> re = table.getElement(i);
    bool highlight = false;
    if (table.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), highlight && focus);
  }
}

void BrowseScreenLocal::update() {
  if (table.size()) {
    Pointer<ResizableElement> re = table.getElement(table.getLastSelectionPointer());
    ui->printStr(re->getRow(), re->getCol(), re->getLabelText());
    re = table.getElement(table.getSelectionPointer());
    ui->printStr(re->getRow(), re->getCol(), re->getLabelText(), focus);
  }
}

void BrowseScreenLocal::command(std::string, std::string) {

}

BrowseScreenAction BrowseScreenLocal::keyPressed(unsigned int ch) {
  switch (ch) {
    case 27: // esc
      ui->returnToLast();
      break;
    case 'c':
    case KEY_LEFT:
      return BrowseScreenAction(BROWSESCREENACTION_CLOSE);
    case KEY_UP:
      if (table.goUp()) {
        ui->update();
      }
      break;
    case KEY_DOWN:
      if (table.goDown()) {
        ui->update();
      }
      break;
    case KEY_RIGHT:
    case 10:
    {
      Pointer<MenuSelectOptionTextButton> msotb = table.getElement(table.getSelectionPointer());
      msotb->getIdentifier();
      break;
    }
  }
  return BrowseScreenAction();
}

std::string BrowseScreenLocal::getLegendText() const {
  return "[Up/Down] Navigate - [Enter/Right] open dir - [Backspace/Left] return - [Esc] Cancel - [c]lose";
}

std::string BrowseScreenLocal::getInfoLabel() const {
  return "LOCAL BROWSING";
}

std::string BrowseScreenLocal::getInfoText() const {
  return "insert path info here";
}

void BrowseScreenLocal::setFocus(bool focus) {
  this->focus = focus;
}
