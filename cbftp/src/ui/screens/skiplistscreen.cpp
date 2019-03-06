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
#include "../misc.h"

SkipListScreen::SkipListScreen(Ui * ui) {
  this->ui = ui;
  table.makeLeavableUp();
  baselegendtext = "[Enter] Modify - [Arrows] select option - [d]one - [c]ancel";
  tablelegendtext = baselegendtext + " - [Del] delete row - [Insert] add row before - [m]ove down - m[o]ve up";
}

SkipListScreen::~SkipListScreen() {

}

void SkipListScreen::initialize(unsigned int row, unsigned int col) {
  globalskip = true;
  this->skiplist = global->getSkipList();
  initialize();
  init(row, col);
}

void SkipListScreen::initialize(unsigned int row, unsigned int col, SkipList * skiplist) {
  globalskip = false;
  this->skiplist = skiplist;
  initialize();
  init(row, col);
}

void SkipListScreen::initialize() {
  currentlegendtext = "";
  active = false;
  table.reset();
  base.reset();
  base.enterFocusFrom(0);
  std::list<SkiplistItem>::const_iterator it;
  testskiplist.clearEntries();
  if (globalskip) {
    testskiplist.setGlobalSkip(NULL);
  }
  else {
    testskiplist.setGlobalSkip(skiplist);
  }
  for (it = skiplist->entriesBegin(); it != skiplist->entriesEnd(); it++) {
    testskiplist.addEntry(it->matchPattern(), it->matchFile(), it->matchDir(), it->matchScope(), it->getAction());
  }
  focusedarea = &base;
  int y = globalskip ? 4 : 5;
  int addx = 1;
  base.addTextButtonNoContent(y++, addx, "add", "<Add pattern>");
  testpattern = base.addStringField(y, 1, "testpattern", "Test pattern:", "", false, 16, 256);
  testtype = base.addTextArrow(y, 34, "testtype", "Test type:");
  testtype->addOption("File", 0);
  testtype->addOption("Dir", 1);
  testtype->setOption(0);
  currentviewspan = 0;
  temphighlightline = -1;
  init(row, col);
}

void SkipListScreen::redraw() {
  ui->erase();
  int y = 1;
  if (!globalskip) {
    ui->printStr(y++, 1, "This skiplist is local and will fall through to the global skiplist if no match is found.");
  }
  ui->printStr(y++, 1, "Valid expressions are * (match any num of any chars except slash) and ? (match any 1 char except slash)");
  ui->printStr(y++, 1, "The pattern list is parsed from top to bottom and the first match applies. Case insensitive.");
  y += 4;
  unsigned int listspan = row - y - 1;
  table.clear();
  std::shared_ptr<ResizableElement> re;
  std::shared_ptr<MenuSelectAdjustableLine> msal = table.addAdjustableLine();
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
    addPatternLine(y, it->matchPattern(), it->matchFile(), it->matchDir(), it->matchScope(), it->getAction());
  }
  if (testskiplist.size()) {
    base.makeLeavableDown();
  }
  else {
    base.makeLeavableDown(false);
  }
  table.adjustLines(col - 3);
  table.checkPointer();
  unsigned int ypos = table.getLineIndex(table.getAdjustableLine(table.getElement(table.getSelectionPointer()))) - 1;
  adaptViewSpan(currentviewspan, listspan, ypos, table.linesSize() - 1);
  bool highlight;
  for (unsigned int i = 0; i < base.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = base.getElement(i);
    highlight = false;
    if (base.getSelectionPointer() == i && &base == focusedarea) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
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
    unsigned int lineindex = table.getLineIndex(table.getAdjustableLine(re));
    ypos = lineindex - 1;
    if (lineindex > 0 && (ypos < currentviewspan || ypos >= currentviewspan + listspan)) {
      continue;
    }
    highlight = false;
    if ((table.getSelectionPointer() == i || (int)re->getRow() == temphighlightline) && &table == focusedarea) {
      highlight = true;
    }
    if (re->isVisible()) {
      ui->printStr(re->getRow() - (lineindex ? currentviewspan : 0), re->getCol(), re->getContentText(), highlight);
    }
  }
  printSlider(ui, row, globalskip ? 8 : 9, col - 1, testskiplist.size(), currentviewspan);
  update();
}

void SkipListScreen::update() {
  if (defocusedarea != NULL) {
    if (defocusedarea == &base) {
      std::shared_ptr<MenuSelectOptionElement> msoe = base.getElement(base.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    }
    else if (defocusedarea == &table) {
      std::shared_ptr<MenuSelectOptionElement> msoe = table.getElement(table.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
    }
    defocusedarea = NULL;
  }
  std::shared_ptr<MenuSelectOptionElement> msoe;
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
    unsigned int ypos = table.getLineIndex(table.getAdjustableLine(table.getElement(table.getSelectionPointer()))) - 1;
    if (ypos < currentviewspan || ypos >= currentviewspan + row - (globalskip ? 8 : 9)) {
      ui->redraw();
      return;
    }
    int lastsel = table.getLastSelectionPointer();
    int sel = table.getSelectionPointer();

    msoe = table.getElement(lastsel);
    ui->printStr(msoe->getRow() - currentviewspan, msoe->getCol(), msoe->getContentText(), false);
    msoe = table.getElement(sel);
    ui->printStr(msoe->getRow() - currentviewspan, msoe->getCol(), msoe->getContentText(), true);
  }
  if (!!msoe) {
    int cursorpos = msoe->cursorPosition();
    if (active && cursorpos >= 0) {
      ui->showCursor();
      unsigned int cursorcol = msoe->getCol() + cursorpos;
      unsigned int cursorrow = msoe->getRow();
      if (focusedarea == &base) {
        cursorcol += msoe->getLabelText().length() + 1;
      }
      else {
        cursorrow -= currentviewspan;
      }
      ui->moveCursor(cursorrow, cursorcol);
    }
    else {
      ui->hideCursor();
    }
  }

  if (col > testtype->getCol() + 20) {
    std::string empty(col - (testtype->getCol() + 20), ' ');
    ui->printStr(testtype->getRow(), testtype->getCol() + 20, empty);
  }
  if (testpattern->getData().length() > 0) {
    std::string allowstring;
    SkipListMatch match = testskiplist.check(testpattern->getData(), testtype->getData());
    if (match.action == SKIPLIST_ALLOW) {
      allowstring = "ALLOWED";
    }
    else if (match.action == SKIPLIST_DENY) {
      allowstring = "DENIED";
    }
    else if (match.action == SKIPLIST_UNIQUE) {
      allowstring = "UNIQUE";
    }
    else if (match.action == SKIPLIST_SIMILAR) {
      allowstring = "SIMILAR";
    }
    ui->printStr(testtype->getRow(), testtype->getCol() + 20, allowstring);
    std::string matchstring = "Match: " + (match.matched ? match.matchpattern : "no match (allowed)");
    ui->printStr(testtype->getRow(), testtype->getCol() + 30, matchstring);
  }
}

bool SkipListScreen::keyPressed(unsigned int ch) {
  if (temphighlightline != -1) {
    temphighlightline = -1;
    ui->redraw();
    if (ch == '-') {
      return true;
    }
  }
  unsigned int pagerows = (row > 8 ? row - 8 : 2) * 0.6;
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
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
      keyUp();
      ui->update();
      return true;
    case KEY_DOWN:
      keyDown();
      ui->update();
      return true;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        keyDown();
      }
      ui->redraw();
      return true;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        keyUp();
      }
      ui->redraw();
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
          addPatternLine(0, "", false, false, SCOPE_IN_RACE, SKIPLIST_ALLOW);
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
        skiplist->addEntry(it->matchPattern(), it->matchFile(), it->matchDir(), it->matchScope(), it->getAction());
      }
      ui->returnToLast();
      return true;
    case KEY_DC:
      if (focusedarea == &table) {
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
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
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        addPatternLine(0, "", false, false, SCOPE_IN_RACE, SKIPLIST_ALLOW, msal);
        saveToTempSkipList();
        ui->redraw();
      }
      return true;
    case 'o':
      if (focusedarea == &table) {
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        if (table.getLineIndex(msal) > 1 && table.swapLineWithPrevious(msal)) {
          table.goUp();
          saveToTempSkipList();
          ui->redraw();
        }
      }
      return true;
    case 'm':
      if (focusedarea == &table) {
        std::shared_ptr<MenuSelectOptionElement> msoe = focusedarea->getElement(focusedarea->getSelectionPointer());
        std::shared_ptr<MenuSelectAdjustableLine> msal = table.getAdjustableLine(msoe);
        if (table.swapLineWithNext(msal)) {
          table.goDown();
          saveToTempSkipList();
          ui->redraw();
        }
      }
      return true;
    case '-':
      if (focusedarea != &table) {
        break;
      }
      temphighlightline = table.getElement(table.getSelectionPointer())->getRow();
      ui->redraw();
      return true;
  }
  return false;
}

void SkipListScreen::keyUp() {
  focusedarea->goUp();
  if (!focusedarea->isFocused()) {
    defocusedarea = focusedarea;
    focusedarea = &base;
    focusedarea->enterFocusFrom(2);
    focusedarea->goLeft();
    ui->setLegend();
  }
}

void SkipListScreen::keyDown() {
  focusedarea->goDown();
  if (!focusedarea->isFocused()) {
    defocusedarea = focusedarea;
    focusedarea = &table;
    focusedarea->enterFocusFrom(0);
    ui->setLegend();
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
  if (globalskip) {
    return "CONFIGURE GLOBAL SKIPLIST";
  }
  else {
    return "CONFIGURE LOCAL SKIPLIST";
  }
}

void SkipListScreen::saveToTempSkipList() {
  std::vector<std::shared_ptr<MenuSelectAdjustableLine> >::iterator it;
  testskiplist.clearEntries();
  for (it = table.linesBegin(); it != table.linesEnd(); it++) {
    if (it == table.linesBegin()) {
      continue;
    }
    std::string pattern = std::static_pointer_cast<MenuSelectOptionTextField>((*it)->getElement(0))->getData();
    bool file = std::static_pointer_cast<MenuSelectOptionCheckBox>((*it)->getElement(1))->getData();
    bool dir = std::static_pointer_cast<MenuSelectOptionCheckBox>((*it)->getElement(2))->getData();
    SkipListAction action = static_cast<SkipListAction>(std::static_pointer_cast<MenuSelectOptionTextArrow>((*it)->getElement(3))->getData());
    int scope = std::static_pointer_cast<MenuSelectOptionTextArrow>((*it)->getElement(4))->getData();
    testskiplist.addEntry(pattern, file, dir, scope, action);
  }
}

void SkipListScreen::addPatternLine(int y, std::string pattern, bool file, bool dir, int scope, SkipListAction action) {
  addPatternLine(y, pattern, file, dir, scope, action, std::shared_ptr<MenuSelectAdjustableLine>());
}

void SkipListScreen::addPatternLine(int y, std::string pattern, bool file, bool dir, int scope, SkipListAction action, std::shared_ptr<MenuSelectAdjustableLine> before) {
  std::shared_ptr<MenuSelectAdjustableLine> msal;
  if (!before) {
    msal = table.addAdjustableLine();
  }
  else {
    msal = table.addAdjustableLineBefore(before);
  }
  std::shared_ptr<ResizableElement> re = table.addStringField(y, 1, "patternedit", "", pattern, false, 64);
  msal->addElement(re, 1, RESIZE_CUTEND, true);
  re = table.addCheckBox(y, 2, "filebox", "", file);
  msal->addElement(re, 2, RESIZE_REMOVE);
  re = table.addCheckBox(y, 3, "dirbox", "", dir);
  msal->addElement(re, 3, RESIZE_REMOVE);
  std::shared_ptr<MenuSelectOptionTextArrow> msota = table.addTextArrow(y, 4, "actionarrow", "");
  msota->addOption("Allow", SKIPLIST_ALLOW);
  msota->addOption("Deny", SKIPLIST_DENY);
  msota->addOption("Unique", SKIPLIST_UNIQUE);
  msota->addOption("Similar", SKIPLIST_SIMILAR);
  msota->setOption(static_cast<int>(action));
  msal->addElement(msota, 4, RESIZE_REMOVE);
  msota = table.addTextArrow(y, 5, "scope", "");
  msota->addOption("In spread job", SCOPE_IN_RACE);
  msota->addOption("Allround", SCOPE_ALL);
  msota->setOption(scope);
  msal->addElement(msota, 5, RESIZE_REMOVE);
}
