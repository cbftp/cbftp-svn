#include "skiplistscreen.h"

#include "../../globalcontext.h"
#include "../../skiplist.h"

#include "../uicommunicator.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncontainer.h"
#include "../termint.h"
#include "../menuselectoptiontextfield.h"

extern GlobalContext * global;

SkipListScreen::SkipListScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  defaultlegendtext = "[Enter] Modify - [Arrows] select option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  unsigned int y = 4;
  unsigned int x = 1;
  skiplist = global->getSkipList();
  mf.initialize(y++, x, skiplist->entriesBegin(), skiplist->entriesEnd());
  mf.enterFocusFrom(0);
  init(window, row, col);
}

void SkipListScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "Files and folders matching the following patterns will be ignored during races.");
  TermInt::printStr(window, 2, 1, "Valid expressions are * (match any num of any chars) and ? (match any 1 char)");
  bool highlight = false;
  int headrow = mf.getHeaderRow();
  int headcol = mf.getHeaderCol();
  unsigned int selected = mf.getSelectionPointer();
  if (mf.isFocused() && selected == 0) {
    highlight = true;
  }
  TermInt::printStr(window, headrow, headcol, "Patterns");
  if (highlight) wattron(window, A_REVERSE);
  TermInt::printStr(window, headrow, headcol + 11, mf.getElement(0)->getContentText());
  if (highlight) wattroff(window, A_REVERSE);
  for (unsigned int i = 0; i < mf.size(); i++) {
    MenuSelectOptionContainer * msoc = mf.getSectionContainer(i);
    for (unsigned int j = 0; j < 2; j++) {
      highlight = ((i * 2) + 1 + j) == selected;
      int indentation = 9;
      if (j == 0) {
        TermInt::printStr(window, headrow + 2 + i, headcol, msoc->getOption(j)->getLabelText());
      }
      if (j == 1) {
        indentation = 74;
      }
      if (highlight) wattron(window, A_REVERSE);
      TermInt::printStr(window, headrow + 2 + i, headcol + indentation, msoc->getOption(j)->getContentText());
      if (highlight) wattroff(window, A_REVERSE);
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
    TermInt::printStr(window, headrow, headcol + 11, mf.getElement(0)->getContentText());
  }
  else {
    MenuSelectOptionContainer * msoc = mf.getSectionContainer((lastsel - 1) / 2);
    int internalid = (lastsel - 1) % 2;
    int add = 9;
    if (internalid == 0) {
      TermInt::printStr(window, headrow + 2 + ((lastsel - 1) / 2), headcol, msoc->getOption(internalid)->getLabelText());
    }
    if (internalid == 1) add = 74;
    TermInt::printStr(window, headrow + 2 + ((lastsel - 1) / 2), headcol + add, msoc->getOption(internalid)->getContentText());
  }
  if (sel == 0) {
    wattron(window, A_REVERSE);
    TermInt::printStr(window, headrow, headcol + 11, mf.getElement(0)->getContentText());
    wattroff(window, A_REVERSE);
  }
  else {
    MenuSelectOptionContainer * msoc = mf.getSectionContainer((sel - 1) / 2);
    int internalid = (sel - 1) % 2;
    int add = 9;
    if (internalid == 0) {
      TermInt::printStr(window, headrow + 2 + ((lastsel - 1) / 2), headcol, msoc->getOption(internalid)->getLabelText());
    }
    if (internalid == 1) add = 74;
    wattron(window, A_REVERSE);
    TermInt::printStr(window, headrow + 2 + ((sel - 1) / 2), headcol + add, msoc->getOption(internalid)->getContentText());
    wattroff(window, A_REVERSE);
    int cursorpos = msoc->getOption(internalid)->cursorPosition();
    if (active && cursorpos >= 0) {
      curs_set(1);
      TermInt::moveCursor(window, headrow + 2 + ((sel - 1) / 2), headcol + add + cursorpos);
    }
    else {
      curs_set(0);
    }
  }
}

void SkipListScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      uicommunicator->newCommand("updatesetlegend");
      return;
    }
    activeelement->inputChar(ch);
    uicommunicator->newCommand("update");
    return;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      mf.goUp();
      uicommunicator->newCommand("update");
      break;
    case KEY_DOWN:
      mf.goDown();
      uicommunicator->newCommand("update");
      break;
    case KEY_LEFT:
      mf.goLeft();
      uicommunicator->newCommand("update");
      break;
    case KEY_RIGHT:
      mf.goRight();
      uicommunicator->newCommand("update");
      break;
    case 10:
      activation = mf.activateSelected();
      if (!activation) {
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = mf.getElement(mf.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 'c':
      uicommunicator->newCommand("return");
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
      uicommunicator->newCommand("return");
      return;
  }
}

std::string SkipListScreen::getLegendText() {
  return currentlegendtext;
}

std::string SkipListScreen::getInfoLabel() {
  return "CONFIGURE SKIPLIST";
}
