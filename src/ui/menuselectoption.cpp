#include "menuselectoption.h"

#include "menuselectoptionelement.h"
#include "menuselectoptiontextfield.h"
#include "menuselectoptionnumarrow.h"
#include "menuselectoptioncheckbox.h"
#include "menuselectoptiontextbutton.h"
#include "menuselectoptiontextarrow.h"
#include "menuselectadjustableline.h"

MenuSelectOption::MenuSelectOption() {
  pointer = 0;
  lastpointer = 0;
}

bool MenuSelectOption::goDown() {
  if (!options.size()) return false;
  unsigned int ccol = options[pointer]->getCol();
  unsigned int crow = options[pointer]->getRow();
  unsigned int closestelem;
  bool movefound = false;
  unsigned int closest = -1;
  for (unsigned int i = 0; i < options.size(); i++) {
    if (!options[i]->visible()) {
      continue;
    }
    unsigned int row = options[i]->getRow();
    if (row > crow && options[i]->getCol() == ccol) {
      if (row < closest || closest == (unsigned int)-1) {
        closest = row;
        closestelem = i;
        movefound = true;
      }
    }
  }
  if (movefound) {
    lastpointer = pointer;
    pointer = closestelem;
    return true;
  }
  if (leavedown) {
    lastpointer = pointer;
    focus = false;
    return true;
  }
  return false;
}

bool MenuSelectOption::goUp() {
  if (!options.size()) return false;
  unsigned int ccol = options[pointer]->getCol();
  unsigned int crow = options[pointer]->getRow();
  unsigned int closestelem;
  bool movefound = false;
  unsigned int closest = -1;
  for (unsigned int i = 0; i < options.size(); i++) {
    if (!options[i]->visible()) {
      continue;
    }
    unsigned int row = options[i]->getRow();
    if (row < crow && options[i]->getCol() == ccol) {
      if (row > closest || closest == (unsigned int)-1) {
        closest = row;
        closestelem = i;
        movefound = true;
      }
    }
  }
  if (movefound) {
    lastpointer = pointer;
    pointer = closestelem;
    return true;
  }
  if (leaveup) {
    lastpointer = pointer;
    focus = false;
    return true;
  }
  return false;
}

bool MenuSelectOption::goRight() {
  if (!options.size()) return false;
  unsigned int ccol = options[pointer]->getCol();
  unsigned int crow = options[pointer]->getRow();
  unsigned int closestelem;
  bool movefound = false;
  unsigned int closest = -1;
  for (unsigned int i = 0; i < options.size(); i++) {
    if (!options[i]->visible()) {
      continue;
    }
    unsigned int col = options[i]->getCol();
    if (col > ccol && options[i]->getRow() == crow) {
      if (col < closest || closest == (unsigned int)-1) {
        closest = col;
        closestelem = i;
        movefound = true;
      }
    }
  }
  if (movefound) {
    lastpointer = pointer;
    pointer = closestelem;
    return true;
  }
  if (leaveright) {
    lastpointer = pointer;
    focus = false;
    return true;
  }
  return false;
}

bool MenuSelectOption::goLeft() {
  if (!options.size()) return false;
  unsigned int ccol = options[pointer]->getCol();
  unsigned int crow = options[pointer]->getRow();
  unsigned int closestelem;
  bool movefound = false;
  unsigned int closest = -1;
  for (unsigned int i = 0; i < options.size(); i++) {
    if (!options[i]->visible()) {
      continue;
    }
    unsigned int col = options[i]->getCol();
    if (col < ccol && options[i]->getRow() == crow) {
      if (col > closest || closest == (unsigned int)-1) {
        closest = col;
        closestelem = i;
        movefound = true;
      }
    }
  }
  if (movefound) {
    lastpointer = pointer;
    pointer = closestelem;
    return true;
  }
  if (leaveleft) {
    lastpointer = pointer;
    focus = false;
    return true;
  }
  return false;
}

bool MenuSelectOption::goNext() {
  if (!options.size()) return false;
  if (pointer < options.size() - 1) {
    lastpointer = pointer;
    pointer++;
    return true;
  }
  return false;
}

bool MenuSelectOption::goPrevious() {
  if (!options.size()) return false;
  if (pointer > 0) {
    lastpointer = pointer;
    pointer--;
    return true;
  }
  return false;
}

void MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret) {
  addStringField(row, col, identifier, label, starttext, secret, 32, 32);
}

void MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret, int maxlen) {
  addStringField(row, col, identifier, label, starttext, secret, maxlen, maxlen);
}

void MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret, int visiblelen, int maxlen) {
  options.push_back(new MenuSelectOptionTextField(identifier, row, col, label, starttext, visiblelen, maxlen, secret));
}

MenuSelectOptionTextArrow * MenuSelectOption::addTextArrow(int row, int col, std::string identifier, std::string label) {
  MenuSelectOptionTextArrow * msota = new MenuSelectOptionTextArrow(identifier, row, col, label);
  options.push_back(msota);
  return msota;
}

void MenuSelectOption::addIntArrow(int row, int col, std::string identifier, std::string label, int startval, int min, int max) {
  options.push_back(new MenuSelectOptionNumArrow(identifier, row, col, label, startval, min, max));
}

void MenuSelectOption::addCheckBox(int row, int col, std::string identifier, std::string label, bool startval) {
  options.push_back(new MenuSelectOptionCheckBox(identifier, row, col, label, startval));
}

MenuSelectOptionTextButton * MenuSelectOption::addTextButton(int row, int col, std::string identifier, std::string label) {
  MenuSelectOptionTextButton * msotb = new MenuSelectOptionTextButton(identifier, row, col, label, true);
  options.push_back(msotb);
  return msotb;
}

MenuSelectOptionTextButton * MenuSelectOption::addTextButtonNoContent(int row, int col, std::string identifier, std::string label) {
  MenuSelectOptionTextButton * msotb = new MenuSelectOptionTextButton(identifier, row, col, label, false);
  options.push_back(msotb);
  return msotb;
}

MenuSelectAdjustableLine * MenuSelectOption::addAdjustableLine() {
  MenuSelectAdjustableLine * msal = new MenuSelectAdjustableLine;
  adjustablelines.push_back(msal);
  return msal;
}

MenuSelectOptionElement * MenuSelectOption::getElement(unsigned int i) const {
  if (i < 0 || i >= size()) {
    return NULL;
  }
  return options[i];
}

MenuSelectOptionElement * MenuSelectOption::getElement(std::string identifier) const {
  std::vector<MenuSelectOptionElement *>::const_iterator it;
  for (it = options.begin(); it != options.end(); it++) {
    if ((*it)->getIdentifier() == identifier) {
      return *it;
    }
  }
  return NULL;
}

unsigned int MenuSelectOption::getLastSelectionPointer() const {
  return lastpointer;
}

unsigned int MenuSelectOption::getSelectionPointer() const {
  return pointer;
}

bool MenuSelectOption::activateSelected() {
  return getElement(pointer)->activate();
}

void MenuSelectOption::clear() {
  std::vector<MenuSelectOptionElement *>::iterator it;
  for (it = options.begin(); it != options.end(); it++) {
    delete *it;
  }
  options.clear();
  std::vector<MenuSelectAdjustableLine *>::iterator it2;
  for (it2 = adjustablelines.begin(); it2 != adjustablelines.end(); it2++) {
    delete *it2;
  }
  adjustablelines.clear();
}

void MenuSelectOption::reset() {
  clear();
  pointer = 0;
  lastpointer = 0;
  focus = false;
}

void MenuSelectOption::enterFocusFrom(int dir) {
  focus = true;
  if (dir == 2) { // bottom
    pointer = size() - 1;
  }
  else {
    pointer = 0;
  }
  lastpointer = pointer;
}

unsigned int MenuSelectOption::size() const {
  return options.size();
}

void MenuSelectOption::adjustLines(unsigned int linesize) {
  if (!adjustablelines.size()) {
    return;
  }
  unsigned int elementcount = adjustablelines[0]->size();
  if (!elementcount) {
    return;
  }
  std::vector<int> maxwantedwidths;
  std::vector<int> maxwidths;
  maxwantedwidths.resize(elementcount); // int is initialized to 0
  maxwidths.resize(elementcount);
  for (std::vector<MenuSelectAdjustableLine *>::iterator it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    for (unsigned int i = 0; i < elementcount; i++) {
      int wantedwidth = (*it)->getElement(i)->wantedWidth();
      if (wantedwidth > maxwantedwidths[i]) {
        maxwantedwidths[i] = wantedwidth;
      }
    }
  }
  unsigned int totalwantedwidth = (maxwantedwidths.size() - 1) * RESIZE_SPACING;
  for (unsigned int i = 0; i < maxwantedwidths.size(); i++) {
    totalwantedwidth += maxwantedwidths[i];
    maxwidths[i] = maxwantedwidths[i];
  }
  while (totalwantedwidth != linesize) {
    if (totalwantedwidth < linesize) {
      for (unsigned int i = 0; i < elementcount; i++) {
        ResizableElement * elem = adjustablelines[0]->getElement(i);
        if (elem->isExpandable()) {
          unsigned int expansion = linesize - totalwantedwidth;
          maxwidths[i] += expansion;
          totalwantedwidth += expansion;
          break;
        }
      }
    }
    else if (totalwantedwidth > linesize) {
      int leastimportant = -1;
      int leastimportantprio = 0;
      for (unsigned int i = 0; i < elementcount; i++) {
        if (!adjustablelines[0]->getElement(i)->isVisible()) {
          continue;
        }
        int prio = adjustablelines[0]->getElement(i)->priority();
        if (prio < leastimportantprio || leastimportant < 0) {
          leastimportantprio = prio;
          leastimportant = i;
        }
      }
      ResizableElement * leastimportantelem = adjustablelines[0]->getElement(leastimportant);
      unsigned int maxsaving = maxwantedwidths[leastimportant] + RESIZE_SPACING;
      unsigned int resizemethod = leastimportantelem->resizeMethod();
      switch (resizemethod) {
        case RESIZE_REMOVE:
          leastimportantelem->setVisible(false);
          totalwantedwidth -= maxsaving;
          break;
        case RESIZE_WITHDOTS:
        case RESIZE_CUTEND:
        case RESIZE_WITHLAST3:
          if (totalwantedwidth - maxwantedwidths[leastimportant] < linesize) {
            int reduction = totalwantedwidth - linesize;
            maxwidths[leastimportant] = maxwantedwidths[leastimportant] - reduction;
            totalwantedwidth -= reduction;
          }
          else {
            leastimportantelem->setVisible(false);
            totalwantedwidth -= maxsaving;
          }
          break;
      }
    }
  }
  int startpos = adjustablelines[0]->getElement(0)->getCol();
  for (std::vector<MenuSelectAdjustableLine *>::iterator it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    int elementpos = startpos;
    for (unsigned int i = 0; i < elementcount; i++) {
      ResizableElement * elem = (*it)->getElement(i);
      if (adjustablelines[0]->getElement(i)->isVisible()) {
        elem->setMaxWidth(maxwidths[i]);
        elem->setPosition(elem->getRow(), elementpos);
        elementpos += maxwidths[i] + RESIZE_SPACING;
      }
      else {
        elem->setVisible(false);
      }
    }
  }
}

void MenuSelectOption::checkPointer() {
  if (pointer >= size()) {
    pointer = size() - 1;
  }
  if (size() == 0) {
    pointer = 0;
  }
}
