#include "skiplistscreen.h"

#include "../../globalcontext.h"
#include "../../skiplist.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncontainer.h"
#include "../menuselectoptiontextfield.h"

extern GlobalContext * global;

SkipListScreen::SkipListScreen(Ui * ui) {
  this->ui = ui;
}

void SkipListScreen::initialize(unsigned int row, unsigned int col) {
  defaultlegendtext = "[Enter] Modify - [Arrows] select option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  unsigned int y = 4;
  unsigned int x = 1;
  skiplist = global->getSkipList();
  mf.initialize(y++, x, skiplist->entriesBegin(), skiplist->entriesEnd());
  mf.enterFocusFrom(0);
  init(row, col);
}

void SkipListScreen::redraw() {
  ui->erase();
  ui->printStr(1, 1, "Files and folders matching the following patterns will be ignored during races.");
  ui->printStr(2, 1, "Valid expressions are * (match any num of any chars) and ? (match any 1 char)");
  bool highlight = false;
  int headrow = mf.getHeaderRow();
  int headcol = mf.getHeaderCol();
  unsigned int selected = mf.getSelectionPointer();
  if (mf.isFocused() && selected == 0) {
    highlight = true;
  }
  ui->printStr(headrow, headcol, "Patterns");
  ui->printStr(headrow, headcol + 11, mf.getElement(0)->getContentText(), highlight);
  for (unsigned int i = 0; i < mf.size(); i++) {
    MenuSelectOptionContainer * msoc = mf.getSectionContainer(i);
    for (unsigned int j = 0; j < 2; j++) {
      highlight = ((i * 2) + 1 + j) == selected;
      int indentation = 9;
      if (j == 0) {
        ui->printStr(headrow + 2 + i, headcol, msoc->getOption(j)->getLabelText());
      }
      if (j == 1) {
        indentation = 74;
      }
      ui->printStr(headrow + 2 + i, headcol + indentation, msoc->getOption(j)->getContentText(), highlight);
    }
  }
}

void SkipListScreen::update() {
  if (mf.needsRedraw()) {
    redraw();
    return;
  }
  int headrow = mf.getHeaderRow();
  int headcol = mf.getHeaderCol();
  int lastsel = mf.getLastSelectionPointer();
  int sel = mf.getSelectionPointer();
  if (lastsel == 0) {
    ui->printStr(headrow, headcol + 11, mf.getElement(0)->getContentText());
  }
  else {
    MenuSelectOptionContainer * msoc = mf.getSectionContainer((lastsel - 1) / 2);
    int internalid = (lastsel - 1) % 2;
    int add = 9;
    if (internalid == 0) {
      ui->printStr(headrow + 2 + ((lastsel - 1) / 2), headcol, msoc->getOption(internalid)->getLabelText());
    }
    if (internalid == 1) add = 74;
    ui->printStr(headrow + 2 + ((lastsel - 1) / 2), headcol + add, msoc->getOption(internalid)->getContentText());
  }
  if (sel == 0) {
    ui->printStr(headrow, headcol + 11, mf.getElement(0)->getContentText(), true);
  }
  else {
    MenuSelectOptionContainer * msoc = mf.getSectionContainer((sel - 1) / 2);
    int internalid = (sel - 1) % 2;
    int add = 9;
    if (internalid == 0) {
      ui->printStr(headrow + 2 + ((lastsel - 1) / 2), headcol, msoc->getOption(internalid)->getLabelText());
    }
    if (internalid == 1) add = 74;
    ui->printStr(headrow + 2 + ((sel - 1) / 2), headcol + add, msoc->getOption(internalid)->getContentText(), true);
    int cursorpos = msoc->getOption(internalid)->cursorPosition();
    if (active && cursorpos >= 0) {
      ui->showCursor();
      ui->moveCursor(headrow + 2 + ((sel - 1) / 2), headcol + add + cursorpos);
    }
    else {
      ui->hideCursor();
    }
  }
}

void SkipListScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      return;
    }
    activeelement->inputChar(ch);
    ui->update();
    return;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      mf.goUp();
      ui->update();
      break;
    case KEY_DOWN:
      mf.goDown();
      ui->update();
      break;
    case KEY_LEFT:
      mf.goLeft();
      ui->update();
      break;
    case KEY_RIGHT:
      mf.goRight();
      ui->update();
      break;
    case 10:
      activation = mf.activateSelected();
      if (!activation) {
        ui->update();
        break;
      }
      active = true;
      activeelement = mf.getElement(mf.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      break;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      break;
    case 'd':
      skiplist->clearEntries();
      for (unsigned int i = 0; i < mf.size(); i++) {
        MenuSelectOptionContainer * msoc = mf.getSectionContainer(i);
        std::string filter = ((MenuSelectOptionTextField *)msoc->getOption(0))->getData();
        if (filter.length() > 0) {
          skiplist->addEntry(filter);
        }
      }
      ui->returnToLast();
      return;
  }
}

std::string SkipListScreen::getLegendText() {
  return currentlegendtext;
}

std::string SkipListScreen::getInfoLabel() {
  return "CONFIGURE SKIPLIST";
}
