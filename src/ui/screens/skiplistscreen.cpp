#include "skiplistscreen.h"

#include "../../globalcontext.h"
#include "../../skiplist.h"

#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncontainer.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextarrow.h"
#include "../resizableelement.h"

extern GlobalContext * global;

SkipListScreen::SkipListScreen(Ui * ui) {
  this->ui = ui;
  table.makeLeavableUp();
  skiplist = global->getSkipList();
  baselegendtext = "[Enter] Modify - [Arrows] select option - [d]one - [c]ancel";
  tablelegendtext = baselegendtext + " - [Del] delete row - [Insert] add row before - [m]ove down - m[o]ve up";
}

void SkipListScreen::initialize(unsigned int row, unsigned int col) {
  currentlegendtext = "";
  active = false;
  table.reset();
  base.reset();
  base.enterFocusFrom(0);
  std::list<SkiplistItem>::const_iterator it;
  testskiplist.clearEntries();
  for (it = skiplist->entriesBegin(); it != skiplist->entriesEnd(); it++) {
    testskiplist.addEntry(it->matchPattern(), it->matchFile(), it->matchDir(), it->isAllowed());
  }
  focusedarea = &base;
  MenuSelectOptionTextArrow * arrow = base.addTextArrow(4, 1, "defaultaction", "Default action:");
  arrow->addOption("Allow", 0);
  arrow->addOption("Deny", 1);
  arrow->setOption(skiplist->defaultAllow() ? 0 : 1);
  base.addTextButtonNoContent(4, 30, "add", "<Add pattern>");
  testpattern = base.addStringField(5, 1, "testpattern", "Test pattern:", "", false, 16, 64);
  testtype = base.addTextArrow(5, 34, "testtype", "Test type:");
  testtype->addOption("File", 0);
  testtype->addOption("Dir", 1);
  testtype->setOption(0);
  init(row, col);
}

void SkipListScreen::redraw() {
  ui->erase();
  int y = 1;
  ui->printStr(y++, 1, "Valid expressions are * (match any num of any chars) and ? (match any 1 char)");
  ui->printStr(y++, 1, "The pattern list is parsed from top to bottom and the first match applies.");
  y = 7;
  table.clear();
  ResizableElement * re;
  MenuSelectAdjustableLine * msal = table.addAdjustableLine();
  re = (ResizableElement *) table.addTextButton(y, 1, "pattern", "PATTERN");
  re->setSelectable(false);
  msal->addElement(re, 1, RESIZE_CUTEND, true);
  re = (ResizableElement *) table.addTextButton(y, 2, "file", "FILE");
  re->setSelectable(false);
  msal->addElement(re, 2, RESIZE_REMOVE);
  re = (ResizableElement *) table.addTextButton(y, 3, "dir", "DIR");
  re->setSelectable(false);
  msal->addElement(re, 3, RESIZE_REMOVE);
  re = (ResizableElement *) table.addTextButton(y, 4, "action", "ACTION");
  re->setSelectable(false);
  msal->addElement(re, 4, RESIZE_REMOVE);
  std::list<SkiplistItem>::const_iterator it;
  for (it = testskiplist.entriesBegin(); it != testskiplist.entriesEnd(); it++) {
    y++;
    addPatternLine(y, it->matchPattern(), it->matchFile(), it->matchDir(), it->isAllowed());
  }
  if (testskiplist.size()) {
    base.makeLeavableDown();
  }
  else {
    base.makeLeavableDown(false);
  }
  table.adjustLines(col - 3);
  table.checkPointer();
  bool highlight;
  for (unsigned int i = 0; i < base.size(); i++) {
    MenuSelectOptionElement * msoe = base.getElement(i);
    highlight = false;
    if (base.getSelectionPointer() == i && &base == focusedarea) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  for (unsigned int i = 0; i < table.size(); i++) {
    ResizableElement * re = (ResizableElement *) table.getElement(i);
    highlight = false;
    if (table.getSelectionPointer() == i && &table == focusedarea) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow(), re->getCol(), re->getContentText(), highlight);
    }
  }
  update();
}

void SkipListScreen::update() {
  if (defocusedarea != NULL) {
    if (defocusedarea == &base) {
      MenuSelectOptionElement * msoe = base.getElement(base.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    }
    else if (defocusedarea == &table) {
      MenuSelectOptionElement * msoe = table.getElement(table.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
    }
    defocusedarea = NULL;
  }
  MenuSelectOptionElement * msoe;
  if (focusedarea == &base) {
    int lastsel = base.getLastSelectionPointer();
    int sel = base.getSelectionPointer();
    msoe = base.getElement(lastsel);
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), false);
    msoe = base.getElement(sel);
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  else if (focusedarea == &table) {
    int lastsel = table.getLastSelectionPointer();
    int sel = table.getSelectionPointer();

    msoe = table.getElement(lastsel);
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText(), false);
    msoe = table.getElement(sel);
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText(), true);
  }
  if (msoe != NULL) {
    int cursorpos = msoe->cursorPosition();
    if (active && cursorpos >= 0) {
      ui->showCursor();
      unsigned int cursoradjust = msoe->getCol() + cursorpos +
          (focusedarea == &base ? msoe->getLabelText().length() + 1 : 0);
      ui->moveCursor(msoe->getRow(), cursoradjust);
    }
    else {
      ui->hideCursor();
    }
  }
  std::string allowstring = "       ";
  if (testpattern->getData().length() > 0) {
    if (testskiplist.isAllowed(testpattern->getData(), testtype->getData())) {
      allowstring = "ALLOWED";
    }
    else {
      allowstring = "DENIED ";
    }
  }
  ui->printStr(testtype->getRow(), testtype->getCol() + 20, allowstring);
}

void SkipListScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      if (activeelement->getIdentifier() == "defaultaction") {
        testskiplist.setDefaultAllow(((MenuSelectOptionTextArrow *)activeelement)->getData() == 0);
      }
      active = false;
      saveToTempSkipList();
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
      focusedarea->goUp();
      if (!focusedarea->isFocused()) {
        defocusedarea = focusedarea;
        focusedarea = &base;
        focusedarea->enterFocusFrom(2);
        focusedarea->goLeft();
        ui->setLegend();
      }
      ui->update();
      break;
    case KEY_DOWN:
      focusedarea->goDown();
      if (!focusedarea->isFocused()) {
        defocusedarea = focusedarea;
        focusedarea = &table;
        focusedarea->enterFocusFrom(0);
        ui->setLegend();
      }
      ui->update();
      break;
    case KEY_LEFT:
      focusedarea->goLeft();
      ui->update();
      break;
    case KEY_RIGHT:
      focusedarea->goRight();
      ui->update();
      break;
    case 10:
      activation = focusedarea->activateSelected();
      if (!activation) {
        if (focusedarea->getElement(focusedarea->getSelectionPointer())->getIdentifier() == "add") {
          addPatternLine(0, "", false, false, true);
          saveToTempSkipList();
          ui->redraw();
          break;
        }
        saveToTempSkipList();
        ui->update();
        break;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
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
      for (std::list<SkiplistItem>::const_iterator it = testskiplist.entriesBegin(); it != testskiplist.entriesEnd(); it++) {
        skiplist->addEntry(it->matchPattern(), it->matchFile(), it->matchDir(), it->isAllowed());
      }
      skiplist->setDefaultAllow(testskiplist.defaultAllow());
      ui->returnToLast();
      return;
    case KEY_DC:
      if (focusedarea == &table) {
        MenuSelectOptionElement * msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        MenuSelectAdjustableLine * msal = table.getAdjustableLine(msoe);
        table.removeAdjustableLine(msal);
        saveToTempSkipList();
        if (testskiplist.size() == 0) {
          table.goUp();
          focusedarea = &base;
          focusedarea->enterFocusFrom(2);
          ui->setLegend();
        }
        ui->redraw();
      }
      break;
    case KEY_IC:
      if (focusedarea == &table) {
        MenuSelectOptionElement * msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        MenuSelectAdjustableLine * msal = table.getAdjustableLine(msoe);
        addPatternLine(0, "", false, false, true, msal);
        saveToTempSkipList();
        ui->redraw();
      }
      break;
    case 'o':
      if (focusedarea == &table) {
        MenuSelectOptionElement * msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        MenuSelectAdjustableLine * msal = table.getAdjustableLine(msoe);
        if (table.getLineIndex(msal) > 1 && table.swapLineWithPrevious(msal)) {
          table.goUp();
          saveToTempSkipList();
          ui->redraw();
        }
      }
      break;
    case 'm':
      if (focusedarea == &table) {
        MenuSelectOptionElement * msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        MenuSelectAdjustableLine * msal = table.getAdjustableLine(msoe);
        if (table.swapLineWithNext(msal)) {
          table.goDown();
          saveToTempSkipList();
          ui->redraw();
        }
      }
      break;
  }
}

std::string SkipListScreen::getLegendText() const {
  if (active) {
    return currentlegendtext;
  }
  else if (focusedarea == &base) {
    return baselegendtext;
  }
  else {
    return tablelegendtext;
  }
}

std::string SkipListScreen::getInfoLabel() const {
  return "CONFIGURE SKIPLIST";
}

void SkipListScreen::saveToTempSkipList() {
  std::vector<MenuSelectAdjustableLine *>::iterator it;
  testskiplist.clearEntries();
  for (it = table.linesBegin(); it != table.linesEnd(); it++) {
    if (it == table.linesBegin()) {
      continue;
    }
    std::string pattern = ((MenuSelectOptionTextField *)(*it)->getElement(0))->getData();
    bool file = ((MenuSelectOptionCheckBox *)(*it)->getElement(1))->getData();
    bool dir = ((MenuSelectOptionCheckBox *)(*it)->getElement(2))->getData();
    bool allow = ((MenuSelectOptionTextArrow *)(*it)->getElement(3))->getData() == 0;
    testskiplist.addEntry(pattern, file, dir, allow);
  }
}

void SkipListScreen::addPatternLine(int y, std::string pattern, bool file, bool dir, bool allow) {
  addPatternLine(y, pattern, file, dir, allow, NULL);
}

void SkipListScreen::addPatternLine(int y, std::string pattern, bool file, bool dir, bool allow, MenuSelectAdjustableLine * before) {
  MenuSelectAdjustableLine * msal;
  if (before == NULL) {
    msal = table.addAdjustableLine();
  }
  else {
    msal = table.addAdjustableLineBefore(before);
  }
  ResizableElement * re = (ResizableElement *) table.addStringField(y, 1, "patternedit", "", pattern, false, 64);
  msal->addElement(re, 1, RESIZE_CUTEND, true);
  re = (ResizableElement *) table.addCheckBox(y, 2, "filebox", "", file);
  msal->addElement(re, 2, RESIZE_REMOVE);
  re = (ResizableElement *) table.addCheckBox(y, 3, "dirbox", "", dir);
  msal->addElement(re, 3, RESIZE_REMOVE);
  MenuSelectOptionTextArrow * msota = table.addTextArrow(y, 3, "actionarrow", "");
  msota->addOption("Allow", 0);
  msota->addOption("Deny", 1);
  msota->setOption(allow ? 0 : 1);
  msal->addElement((ResizableElement *)msota, 4, RESIZE_REMOVE);
}
