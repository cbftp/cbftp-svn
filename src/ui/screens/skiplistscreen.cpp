#include "skiplistscreen.h"

#include "../../globalcontext.h"

#include "../ui.h"
#include "../focusablearea.h"
#include "../menuselectoptioncontainer.h"
#include "../menuselectadjustableline.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
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

SkipListScreen::~SkipListScreen() {

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
    testskiplist.addEntry(it->matchPattern(), it->matchFile(), it->matchDir(), it->matchScope(), it->isAllowed());
  }
  focusedarea = &base;
  Pointer<MenuSelectOptionTextArrow> arrow = base.addTextArrow(4, 1, "defaultaction", "Default action:");
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
  ui->printStr(y++, 1, "Valid expressions are * (match any num of any chars except slash) and ? (match any 1 char except slash)");
  ui->printStr(y++, 1, "The pattern list is parsed from top to bottom and the first match applies.");
  y = 7;
  table.clear();
  Pointer<ResizableElement> re;
  Pointer<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
  re = table.addTextButton(y, 1, "pattern", "PATTERN");
  re->setSelectable(false);
  msal->addElement(re, 1, RESIZE_CUTEND, true);
  re = table.addTextButton(y, 2, "file", "FILE");
  re->setSelectable(false);
  msal->addElement(re, 2, RESIZE_REMOVE);
  re = table.addTextButton(y, 3, "dir", "DIR");
  re->setSelectable(false);
  msal->addElement(re, 3, RESIZE_REMOVE);
  re = table.addTextButton(y, 4, "action", "ACTION");
  re->setSelectable(false);
  msal->addElement(re, 4, RESIZE_REMOVE);
  re = table.addTextButton(y, 5, "scope", "SCOPE");
  re->setSelectable(false);
  msal->addElement(re, 5, RESIZE_REMOVE);
  std::list<SkiplistItem>::const_iterator it;
  for (it = testskiplist.entriesBegin(); it != testskiplist.entriesEnd(); it++) {
    y++;
    addPatternLine(y, it->matchPattern(), it->matchFile(), it->matchDir(), it->matchScope(), it->isAllowed());
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
    Pointer<MenuSelectOptionElement> msoe = base.getElement(i);
    highlight = false;
    if (base.getSelectionPointer() == i && &base == focusedarea) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
  for (unsigned int i = 0; i < table.size(); i++) {
    Pointer<ResizableElement> re = table.getElement(i);
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
      Pointer<MenuSelectOptionElement> msoe = base.getElement(base.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    }
    else if (defocusedarea == &table) {
      Pointer<MenuSelectOptionElement> msoe = table.getElement(table.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
    }
    defocusedarea = NULL;
  }
  Pointer<MenuSelectOptionElement> msoe;
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
  if (!!msoe) {
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

bool SkipListScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      if (activeelement->getIdentifier() == "defaultaction") {
        testskiplist.setDefaultAllow(activeelement.get<MenuSelectOptionTextArrow>()->getData() == 0);
      }
      active = false;
      saveToTempSkipList();
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
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
      return true;
    case KEY_DOWN:
      focusedarea->goDown();
      if (!focusedarea->isFocused()) {
        defocusedarea = focusedarea;
        focusedarea = &table;
        focusedarea->enterFocusFrom(0);
        ui->setLegend();
      }
      ui->update();
      return true;
    case KEY_LEFT:
      focusedarea->goLeft();
      ui->update();
      return true;
    case KEY_RIGHT:
      focusedarea->goRight();
      ui->update();
      return true;
    case 10:
      activation = focusedarea->activateSelected();
      if (!activation) {
        if (focusedarea->getElement(focusedarea->getSelectionPointer())->getIdentifier() == "add") {
          addPatternLine(0, "", false, false, SCOPE_IN_RACE, true);
          saveToTempSkipList();
          ui->redraw();
          return true;
        }
        saveToTempSkipList();
        ui->update();
        return true;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      return true;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
    case 'd':
      skiplist->clearEntries();
      for (std::list<SkiplistItem>::const_iterator it = testskiplist.entriesBegin(); it != testskiplist.entriesEnd(); it++) {
        skiplist->addEntry(it->matchPattern(), it->matchFile(), it->matchDir(), it->matchScope(), it->isAllowed());
      }
      skiplist->setDefaultAllow(testskiplist.defaultAllow());
      ui->returnToLast();
      return true;
    case KEY_DC:
      if (focusedarea == &table) {
        Pointer<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        Pointer<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
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
      return true;
    case KEY_IC:
      if (focusedarea == &table) {
        Pointer<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        Pointer<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        addPatternLine(0, "", false, false, SCOPE_IN_RACE, true, msal);
        saveToTempSkipList();
        ui->redraw();
      }
      return true;
    case 'o':
      if (focusedarea == &table) {
        Pointer<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        Pointer<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        if (table.getLineIndex(msal) > 1 && table.swapLineWithPrevious(msal)) {
          table.goUp();
          saveToTempSkipList();
          ui->redraw();
        }
      }
      return true;
    case 'm':
      if (focusedarea == &table) {
        Pointer<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        Pointer<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        if (table.swapLineWithNext(msal)) {
          table.goDown();
          saveToTempSkipList();
          ui->redraw();
        }
      }
      return true;
  }
  return false;
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
  std::vector<Pointer<MenuSelectAdjustableLine> >::iterator it;
  testskiplist.clearEntries();
  for (it = table.linesBegin(); it != table.linesEnd(); it++) {
    if (it == table.linesBegin()) {
      continue;
    }
    std::string pattern = (*it)->getElement(0).get<MenuSelectOptionTextField>()->getData();
    bool file = (*it)->getElement(1).get<MenuSelectOptionCheckBox>()->getData();
    bool dir = (*it)->getElement(2).get<MenuSelectOptionCheckBox>()->getData();
    bool allow = (*it)->getElement(3).get<MenuSelectOptionTextArrow>()->getData() == 0;
    int scope = (*it)->getElement(4).get<MenuSelectOptionTextArrow>()->getData();
    testskiplist.addEntry(pattern, file, dir, scope, allow);
  }
}

void SkipListScreen::addPatternLine(int y, std::string pattern, bool file, bool dir, int scope, bool allow) {
  addPatternLine(y, pattern, file, dir, scope, allow, Pointer<MenuSelectAdjustableLine>());
}

void SkipListScreen::addPatternLine(int y, std::string pattern, bool file, bool dir, int scope, bool allow, Pointer<MenuSelectAdjustableLine> before) {
  Pointer<MenuSelectAdjustableLine> msal;
  if (!before) {
    msal = table.addAdjustableLine();
  }
  else {
    msal = table.addAdjustableLineBefore(before);
  }
  Pointer<ResizableElement> re = table.addStringField(y, 1, "patternedit", "", pattern, false, 64);
  msal->addElement(re, 1, RESIZE_CUTEND, true);
  re = table.addCheckBox(y, 2, "filebox", "", file);
  msal->addElement(re, 2, RESIZE_REMOVE);
  re = table.addCheckBox(y, 3, "dirbox", "", dir);
  msal->addElement(re, 3, RESIZE_REMOVE);
  Pointer<MenuSelectOptionTextArrow> msota = table.addTextArrow(y, 4, "actionarrow", "");
  msota->addOption("Allow", 0);
  msota->addOption("Deny", 1);
  msota->setOption(allow ? 0 : 1);
  msal->addElement(msota, 4, RESIZE_REMOVE);
  msota = table.addTextArrow(y, 5, "scope", "");
  msota->addOption("In race", SCOPE_IN_RACE);
  msota->addOption("Allround", SCOPE_ALL);
  msota->setOption(scope);
  msal->addElement(msota, 5, RESIZE_REMOVE);
}
