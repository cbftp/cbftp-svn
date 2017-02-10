#include "menuselectoption.h"

#include <cstdlib>

#include "menuselectoptionelement.h"
#include "menuselectoptiontextfield.h"
#include "menuselectoptionnumarrow.h"
#include "menuselectoptioncheckbox.h"
#include "menuselectoptiontextbutton.h"
#include "menuselectoptiontextarrow.h"
#include "menuselectadjustableline.h"

#define MAX_DIFF_LR 30
#define MAX_DIFF_UD 2

enum Direction {
  UP,
  DOWN,
  LEFT,
  RIGHT
};

MenuSelectOption::MenuSelectOption() {
  pointer = 0;
  lastpointer = 0;
}

MenuSelectOption::~MenuSelectOption() {

}

bool MenuSelectOption::navigate(int dir) {
  if (!options.size()) return false;
  int ccol = options[pointer]->getCol();
  int crow = options[pointer]->getRow();
  unsigned int closestelem;
  bool movefound = false;
  int closest = -1;
  for (unsigned int i = 0; i < options.size(); i++) {
    if (!options[i]->visible() || !options[i]->isSelectable()) {
      continue;
    }
    int row = options[i]->getRow();
    int col = options[i]->getCol();
    int distance = abs(row - crow) + abs(col - ccol) * 2;
    if ((((dir == DOWN && row > crow) || (dir == UP && row < crow)) &&
         abs(col - ccol) <= MAX_DIFF_LR) ||
        (((dir == LEFT && col < ccol) || (dir == RIGHT && col > ccol)) &&
         abs(row - crow) <= MAX_DIFF_UD))
      {
      if (distance < closest || closest == -1) {
        closest = distance;
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
  if ((dir == DOWN && leavedown) || (dir == UP && leaveup) ||
      (dir == LEFT && leaveleft) || (dir == RIGHT && leaveright))
    {
    lastpointer = pointer;
    focus = false;
    return true;
  }
  return false;
}

bool MenuSelectOption::goDown() {
  return navigate(DOWN);

}

bool MenuSelectOption::goUp() {
  return navigate(UP);
}

bool MenuSelectOption::goRight() {
  return navigate(RIGHT);
}

bool MenuSelectOption::goLeft() {
  return navigate(LEFT);
}

bool MenuSelectOption::goNext() {
  if (!options.size()) return false;
  unsigned int temppointer = pointer;
  while (pointer < options.size() - 1) {
    pointer++;
    if (options[pointer]->isSelectable()) {
      lastpointer = temppointer;
      return true;
    }
  }
  return false;
}

bool MenuSelectOption::goPrevious() {
  if (!options.size()) return false;
  unsigned int temppointer = pointer;
  while (pointer > 0) {
    pointer--;
    if (options[pointer]->isSelectable()) {
      lastpointer = temppointer;
      return true;
    }
  }
  return false;
}

Pointer<MenuSelectOptionTextField> MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret) {
  return addStringField(row, col, identifier, label, starttext, secret, 32, 32);
}

Pointer<MenuSelectOptionTextField> MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret, int maxlen) {
  return addStringField(row, col, identifier, label, starttext, secret, maxlen, maxlen);
}

Pointer<MenuSelectOptionTextField> MenuSelectOption::addStringField(int row, int col, std::string identifier, std::string label, std::string starttext, bool secret, int visiblelen, int maxlen) {
  Pointer<MenuSelectOptionTextField> msotf(makePointer<MenuSelectOptionTextField>(identifier, row, col, label, starttext, visiblelen, maxlen, secret));
  options.push_back(msotf);
  return msotf;
}

Pointer<MenuSelectOptionTextArrow> MenuSelectOption::addTextArrow(int row, int col, std::string identifier, std::string label) {
  Pointer<MenuSelectOptionTextArrow> msota(makePointer<MenuSelectOptionTextArrow>(identifier, row, col, label));
  options.push_back(msota);
  return msota;
}

Pointer<MenuSelectOptionNumArrow> MenuSelectOption::addIntArrow(int row, int col, std::string identifier, std::string label, int startval, int min, int max) {
  Pointer<MenuSelectOptionNumArrow> msona(makePointer<MenuSelectOptionNumArrow>(identifier, row, col, label, startval, min, max));
  options.push_back(msona);
  return msona;
}

Pointer<MenuSelectOptionCheckBox> MenuSelectOption::addCheckBox(int row, int col, std::string identifier, std::string label, bool startval) {
  Pointer<MenuSelectOptionCheckBox> msocb(makePointer<MenuSelectOptionCheckBox>(identifier, row, col, label, startval));
  options.push_back(msocb);
  return msocb;
}

Pointer<MenuSelectOptionTextButton> MenuSelectOption::addTextButton(int row, int col, std::string identifier, std::string label) {
  Pointer<MenuSelectOptionTextButton> msotb(makePointer<MenuSelectOptionTextButton>(identifier, row, col, label, true));
  options.push_back(msotb);
  return msotb;
}

Pointer<MenuSelectOptionTextButton> MenuSelectOption::addTextButtonNoContent(int row, int col, std::string identifier, std::string label) {
  Pointer<MenuSelectOptionTextButton> msotb(makePointer<MenuSelectOptionTextButton>(identifier, row, col, label, false));
  options.push_back(msotb);
  return msotb;
}

Pointer<MenuSelectAdjustableLine> MenuSelectOption::addAdjustableLine() {
  Pointer<MenuSelectAdjustableLine> msal(makePointer<MenuSelectAdjustableLine>());
  adjustablelines.push_back(msal);
  return msal;
}

Pointer<MenuSelectAdjustableLine> MenuSelectOption::addAdjustableLineBefore(Pointer<MenuSelectAdjustableLine> before) {
  Pointer<MenuSelectAdjustableLine> msal(makePointer<MenuSelectAdjustableLine>());
  std::vector<Pointer<MenuSelectAdjustableLine> >::iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == before) {
      adjustablelines.insert(it, msal);
      return msal;
    }
  }
  adjustablelines.push_back(msal);
  return msal;
}

Pointer<MenuSelectOptionElement> MenuSelectOption::getElement(unsigned int i) const {
  if (i >= size()) {
    return Pointer<MenuSelectOptionElement>();
  }
  return options[i];
}

Pointer<MenuSelectOptionElement> MenuSelectOption::getElement(std::string identifier) const {
  std::vector<Pointer<MenuSelectOptionElement> >::const_iterator it;
  for (it = options.begin(); it != options.end(); it++) {
    if ((*it)->getIdentifier() == identifier) {
      return *it;
    }
  }
  return Pointer<MenuSelectOptionElement>();
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
  options.clear();
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
  if (lastpointer) {
    pointer = lastpointer;
  }
  else if (dir == 2) {
    pointer = lastpointer = size() - 1;
  }
  checkPointer();
}

unsigned int MenuSelectOption::size() const {
  return options.size();
}

unsigned int MenuSelectOption::linesSize() const {
  return adjustablelines.size();
}

void MenuSelectOption::adjustLines(unsigned int linesize) {
  if (!adjustablelines.size()) {
    return;
  }
  unsigned int elementcount = adjustablelines[0]->size();
  if (!elementcount) {
    return;
  }
  std::vector<unsigned int> maxwantedwidths;
  std::vector<unsigned int> maxwidths;
  std::vector<unsigned int> averagewantedwidths;
  maxwantedwidths.resize(elementcount); // int is initialized to 0
  maxwidths.resize(elementcount);
  averagewantedwidths.resize(elementcount);
  int shortspaces = 0;
  for (std::vector<Pointer<MenuSelectAdjustableLine> >::iterator it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    for (unsigned int i = 0; i < elementcount; i++) {
      Pointer<ResizableElement> re = (*it)->getElement(i);
      unsigned int wantedwidth = re->wantedWidth();
      if (wantedwidth > maxwantedwidths[i]) {
        maxwantedwidths[i] = wantedwidth;
      }
      averagewantedwidths[i] += wantedwidth;
      if (it == adjustablelines.begin() && re->shortSpacing() && i + 1 != elementcount) {
        shortspaces++;
      }
    }
  }
  unsigned int totalwantedwidth = (maxwantedwidths.size() - 1) * RESIZE_SPACING -
      shortspaces * (RESIZE_SPACING - RESIZE_SPACING_SHORT);
  for (unsigned int i = 0; i < maxwantedwidths.size(); i++) {
    totalwantedwidth += maxwantedwidths[i];
    maxwidths[i] = maxwantedwidths[i];
    averagewantedwidths[i] = (averagewantedwidths[i] * 0.9) / adjustablelines.size();
  }
  while (totalwantedwidth != linesize) {
    if (totalwantedwidth < linesize) {
      bool expanded = false;
      for (unsigned int i = 0; i < elementcount; i++) {
        Pointer<ResizableElement> elem = adjustablelines[0]->getElement(i);
        if (elem->isExpandable()) {
          unsigned int expansion = linesize - totalwantedwidth;
          maxwidths[i] += expansion;
          totalwantedwidth += expansion;
          expanded = true;
          break;
        }
      }
      if (!expanded) {
        break;
      }
    }
    else if (totalwantedwidth > linesize) {
      int leastimportant = -1;
      int leastimportantprio = 0;
      bool leastimportantpartialremove;
      bool leastimportanthighprio;
      for (unsigned int i = 0; i < elementcount; i++) {
        if (!adjustablelines[0]->getElement(i)->isVisible()) {
          continue;
        }
        Pointer<ResizableElement> re = adjustablelines[0]->getElement(i);
        int prio = re->lowPriority();
        int highprio = re->highPriority();
        bool partialremove = false;
        bool athighprio = false;
        if (prio != highprio) {
          partialremove = true;
          if (maxwidths[i] <= averagewantedwidths[i]) {
            athighprio = true;
            prio = highprio;
            partialremove = false;
          }
        }
        if (prio < leastimportantprio || leastimportant < 0) {
          leastimportantprio = prio;
          leastimportant = i;
          leastimportantpartialremove = partialremove;
          leastimportanthighprio = athighprio;
        }
      }
      Pointer<ResizableElement> leastimportantelem = adjustablelines[0]->getElement(leastimportant);
      int spacing = leastimportantelem->shortSpacing() ? RESIZE_SPACING_SHORT : RESIZE_SPACING;
      unsigned int maxsaving = maxwantedwidths[leastimportant] + spacing;
      unsigned int resizemethod = leastimportantelem->resizeMethod();
      switch (resizemethod) {
        case RESIZE_REMOVE:
          leastimportantelem->setVisible(false);
          totalwantedwidth -= maxsaving;
          break;
        case RESIZE_WITHDOTS:
        case RESIZE_CUTEND:
        case RESIZE_WITHLAST3: {
          int maxwantedwidth = (leastimportanthighprio ? averagewantedwidths : maxwantedwidths)[leastimportant];
          if (leastimportantpartialremove || totalwantedwidth - maxwantedwidth < linesize) {
            int reduction = leastimportantpartialremove ?
                maxwantedwidth - averagewantedwidths[leastimportant] :
                totalwantedwidth - linesize;
            maxwidths[leastimportant] = maxwantedwidth - reduction;
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
  }
  int startpos = adjustablelines[0]->getElement(0)->getCol();
  for (std::vector<Pointer<MenuSelectAdjustableLine> >::iterator it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    int elementpos = startpos;
    for (unsigned int i = 0; i < elementcount; i++) {
      Pointer<ResizableElement> elem = (*it)->getElement(i);
      if (adjustablelines[0]->getElement(i)->isVisible()) {
        elem->setMaxWidth(maxwidths[i]);
        elem->setPosition(elem->getRow(), elementpos);
        int spacing = elem->shortSpacing() ? RESIZE_SPACING_SHORT : RESIZE_SPACING;
        elementpos += maxwidths[i] + spacing;
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
  else {
    while ((!options[pointer]->visible() || !options[pointer]->isSelectable()) && pointer > 0) pointer--;
    while ((!options[pointer]->visible() || !options[pointer]->isSelectable()) && pointer < size() - 1) pointer++;
  }
  lastpointer = pointer;
}

std::vector<Pointer<MenuSelectAdjustableLine> >::iterator MenuSelectOption::linesBegin() {
  return adjustablelines.begin();
}

std::vector<Pointer<MenuSelectAdjustableLine> >::iterator MenuSelectOption::linesEnd() {
  return adjustablelines.end();
}

Pointer<MenuSelectAdjustableLine> MenuSelectOption::getAdjustableLine(Pointer<MenuSelectOptionElement> msoe) const {
  std::vector<Pointer<MenuSelectAdjustableLine> >::const_iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    for (unsigned int i = 0; i < (*it)->size(); i++) {
      if ((*it)->getElement(i) == msoe) {
        return *it;
      }
    }
  }
  return Pointer<MenuSelectAdjustableLine>();
}

void MenuSelectOption::removeAdjustableLine(Pointer<MenuSelectAdjustableLine> msal) {
  std::vector<Pointer<MenuSelectAdjustableLine> >::iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == msal) {
      for (unsigned int i = 0; i < (*it)->size(); i++) {
        removeElement(msal->getElement(i));
      }
      adjustablelines.erase(it);
      return;
    }
  }
}

void MenuSelectOption::removeElement(Pointer<MenuSelectOptionElement> msoe) {
  std::vector<Pointer<MenuSelectOptionElement> >::iterator it;
  for (it = options.begin(); it != options.end(); it++) {
    if (*it == msoe) {
      options.erase(it);
      return;
    }
  }
}

void MenuSelectOption::setPointer(Pointer<MenuSelectOptionElement> set) {
  for (unsigned int i = 0; i < options.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = options[i];
    if (msoe == set) {
      pointer = i;
      return;
    }
  }
}

bool MenuSelectOption::swapLineWithNext(Pointer<MenuSelectAdjustableLine> msal) {
  std::vector<Pointer<MenuSelectAdjustableLine> >::iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == msal) {
      if (it + 1 != adjustablelines.end()) {
        Pointer<MenuSelectAdjustableLine> swap = *(it + 1);
        *(it + 1) = msal;
        *it = swap;
        return true;
      }
      return false;
    }
  }
  return false;
}

bool MenuSelectOption::swapLineWithPrevious(Pointer<MenuSelectAdjustableLine> msal) {
  std::vector<Pointer<MenuSelectAdjustableLine> >::iterator it;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == msal) {
      if (it != adjustablelines.begin()) {
        Pointer<MenuSelectAdjustableLine> swap = *(it - 1);
        *(it - 1) = msal;
        *it = swap;
        return true;
      }
      return false;
    }
  }
  return false;
}

int MenuSelectOption::getLineIndex(Pointer<MenuSelectAdjustableLine> msal) {
  std::vector<Pointer<MenuSelectAdjustableLine> >::iterator it;
  int index = 0;
  for (it = adjustablelines.begin(); it != adjustablelines.end(); it++) {
    if (*it == msal) {
      return index;
    }
    index++;
  }
  return -1;
}
