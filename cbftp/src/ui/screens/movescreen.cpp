#include "movescreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../../path.h"

MoveScreen::MoveScreen(Ui * ui) {
  this->ui = ui;
}

MoveScreen::~MoveScreen() {

}

void MoveScreen::initialize(unsigned int row, unsigned int col, const std::string & site, const std::string& items, const Path& srcpath, const std::string& dstpath) {
  defaultlegendtext = "[Enter] Modify - [m]ove - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  this->site = site;
  this->srcpath = srcpath.toString();
  this->items = items;
  mso.reset();
  int y = 4;
  if (!site.empty()) {
    y++;
  }
  mso.addStringField(y, 1, "dstpath", "Target path/name:", dstpath, false, col - 3, 512);
  mso.enterFocusFrom(0);
  init(row, col);
}

void MoveScreen::redraw() {
  ui->erase();
  int y = 1;
  if (!site.empty()) {
    ui->printStr(y++, 1, "Site: " + site);
  }
  ui->printStr(y++, 1, "Source path: " + srcpath);
  ui->printStr(y++, 1, "Item: " + items);
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  if (active && activeelement->cursorPosition() >= 0) {
    ui->showCursor();
    ui->moveCursor(activeelement->getRow(), activeelement->getCol() + activeelement->getLabelText().length() + 1 + activeelement->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void MoveScreen::update() {
  redraw();
}

bool MoveScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  switch(ch) {
    case 10:
      activeelement = mso.getElement(mso.getSelectionPointer());
      activeelement->activate();
      active = true;
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      return true;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
    case 'd':
    case 'm': {
      std::string dstpath = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("dstpath"))->getData();
      if (!dstpath.empty()) {
        ui->returnMove(std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("dstpath"))->getData());
      }
      return true;
    }
  }
  return false;
}

std::string MoveScreen::getLegendText() const {
  return currentlegendtext;
}

std::string MoveScreen::getInfoLabel() const {
  return "MOVE/RENAME";
}
