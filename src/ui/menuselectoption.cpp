#include "menuselectoption.h"

#include "menuselectoptionelement.h"
#include "menuselectoptiontextfield.h"
#include "menuselectoptionnumarrow.h"
#include "menuselectoptioncheckbox.h"
#include "menuselectoptiontextbutton.h"
#include "menuselectoptiontextarrow.h"

MenuSelectOption::MenuSelectOption() {
  pointer = 0;
  lastpointer = 0;
}

bool MenuSelectOption::goDown() {
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

void MenuSelectOption::addTextButton(int row, int col, std::string identifier, std::string label) {
  options.push_back(new MenuSelectOptionTextButton(identifier, row, col, label, true));
}

void MenuSelectOption::addTextButtonNoContent(int row, int col, std::string identifier, std::string label) {
  options.push_back(new MenuSelectOptionTextButton(identifier, row, col, label, false));
}

MenuSelectOptionElement * MenuSelectOption::getElement(unsigned int i) {
  if (i < 0 || i >= size()) {
    return NULL;
  }
  return options[i];
}

MenuSelectOptionElement * MenuSelectOption::getElement(std::string identifier) {
  std::vector<MenuSelectOptionElement *>::iterator it;
  for (it = options.begin(); it != options.end(); it++) {
    if ((*it)->getIdentifier() == identifier) {
      return *it;
    }
  }
  return NULL;
}

unsigned int MenuSelectOption::getLastSelectionPointer() {
  return lastpointer;
}

unsigned int MenuSelectOption::getSelectionPointer() {
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

unsigned int MenuSelectOption::size() {
  return options.size();
}

void MenuSelectOption::checkPointer() {
  if (pointer >= size()) {
    pointer = size() - 1;
  }
  if (size() == 0) {
    pointer = 0;
  }
}
