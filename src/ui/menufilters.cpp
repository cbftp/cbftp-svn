#include "menufilters.h"

#include "menuselectoptioncontainer.h"
#include "menuselectoptionelement.h"
#include "menuselectoptiontextfield.h"
#include "menuselectoptiontextbutton.h"

MenuFilters::MenuFilters() {
  pointer = 0;
  lastpointer = 0;
  needsredraw = false;
}

void MenuFilters::initialize(int row, int col, std::list<std::string>::iterator sectionsbegin, std::list<std::string>::iterator sectionsend) {
  this->row = row;
  this->col = col;
  std::list<std::string>::iterator it;
  addbutton = new MenuSelectOptionTextButton("add", 0, 0, "<Add>");
  for(it = sectionsbegin; it != sectionsend; it++) {
    addFilter(*it);
  }
}

bool MenuFilters::goDown() {
  if (pointer > 0) {
    if (pointer + 2 <= size() * 2) {
      lastpointer = pointer;
      pointer = pointer + 2;
      return true;
    }
    else if (leavedown) {
      lastpointer = pointer;
      focus = false;
      return true;
    }
  }
  else if (pointer == 0 && size() > 0) {
    lastpointer = pointer;
    pointer++;
    return true;
  }
  return false;
}

bool MenuFilters::goUp() {
  if (pointer > 0 && pointer < 2) {
    lastpointer = pointer;
    pointer = 0;
    return true;
  }
  else if (pointer >= 2) {
    lastpointer = pointer;
    pointer = pointer - 2;
    return true;
  }
  else if (leaveup) {
    lastpointer = pointer;
    focus = false;
    return true;
  }
  return false;
}

bool MenuFilters::goRight() {
  if (pointer > 0 && (pointer - 1) % 2 < 1) {
    lastpointer = pointer;
    pointer++;
    return true;
  }
  return false;
}

bool MenuFilters::goLeft() {
  if (pointer > 0 && (pointer - 1) % 2 > 0) {
    lastpointer = pointer;
    pointer--;
    return true;
  }
  return false;
}

MenuSelectOptionContainer * MenuFilters::getSectionContainer(unsigned int id) {
  if (id <= filtercontainers.size() - 1) {
    return &filtercontainers[id];
  }
  return NULL;
}

unsigned int MenuFilters::getLastSelectionPointer() {
  return lastpointer;
}

unsigned int MenuFilters::getSelectionPointer() {
  return pointer;
}

MenuSelectOptionElement * MenuFilters::getElement(unsigned int i) {
  if (i == 0) {
    return addbutton;
  }
  if (i < 0 || i > filtercontainers.size() * 2) {
    return NULL;
  }
  int id = (i - 1) / 2;
  int internalid = (i - 1) % 2;
  return filtercontainers[id].getOption(internalid);
}

bool MenuFilters::activateSelected() {
  MenuSelectOptionElement * msoe = getElement(pointer);
  if (msoe->getIdentifier() == "add") {
    addFilter();
    needsredraw = true;
    return false;
  }
  else if (msoe->getIdentifier() == "delete") {
    filtercontainers.erase(filtercontainers.begin() + ((pointer - 1) / 2));
    lastpointer = pointer;
    if (getElement(pointer) == NULL) {
      goUp();
    }
    needsredraw = true;
    return false;
  }
  return msoe->activate();
}

bool MenuFilters::needsRedraw() {
  bool redraw = needsredraw;
  needsredraw = false;
  return redraw;
}

unsigned int MenuFilters::getHeaderRow() {
  return row;
}

unsigned int MenuFilters::getHeaderCol() {
  return col;
}

unsigned int MenuFilters::size() {
  return filtercontainers.size();
}

bool MenuFilters::addButtonPressed() {
  return true;
}

void MenuFilters::addFilter(std::string filtertext) {
  MenuSelectOptionElement * filter = new MenuSelectOptionTextField("filter", 0, 0, "Pattern:", filtertext, 64, 64, false);
  MenuSelectOptionElement * del = new MenuSelectOptionTextButton("delete", 0, 0, "<X>");
  MenuSelectOptionContainer msoc = MenuSelectOptionContainer();
  msoc.addElement(filter);
  msoc.addElement(del);
  filtercontainers.push_back(msoc);
}
void MenuFilters::addFilter() {
  addFilter("");
}

void MenuFilters::enterFocusFrom(int dir) {
  focus = true;
  pointer = 0;
}
