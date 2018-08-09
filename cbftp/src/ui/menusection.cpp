#include "menusection.h"

#include "menuselectoptiontextfield.h"
#include "menuselectoptiontextbutton.h"
#include "menuselectoptionelement.h"

#include "../path.h"

MenuSection::MenuSection() {
  pointer = 0;
  lastpointer = 0;
  needsredraw = false;
}

MenuSection::~MenuSection() {

}

void MenuSection::initialize(int row, int col, std::map<std::string, Path>::const_iterator sectionsbegin, std::map<std::string, Path>::const_iterator sectionsend) {
  pointer = 0;
  lastpointer = 0;
  needsredraw = false;
  focus = false;
  sectioncontainers.clear();
  this->row = row;
  this->col = col;
  std::map<std::string, Path>::const_iterator it;
  addbutton = std::make_shared<MenuSelectOptionTextButton>("add", 0, 0, "<Add>");
  for(it = sectionsbegin; it != sectionsend; it++) {
    addSection(it->first, it->second);
  }
}

bool MenuSection::goDown() {
  if (pointer > 0) {
    if (pointer + 3 <= size() * 3) {
      lastpointer = pointer;
      pointer = pointer + 3;
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

bool MenuSection::goUp() {
  if (pointer > 0 && pointer < 3) {
    lastpointer = pointer;
    pointer = 0;
    return true;
  }
  else if (pointer >= 3) {
    lastpointer = pointer;
    pointer = pointer - 3;
    return true;
  }
  else if (leaveup) {
    lastpointer = pointer;
    focus = false;
    return true;
  }
  return false;
}

bool MenuSection::goRight() {
  if (pointer > 0 && (pointer - 1) % 3 < 2) {
    lastpointer = pointer;
    pointer++;
    return true;
  }
  return false;
}

bool MenuSection::goLeft() {
  if (pointer > 0 && (pointer - 1) % 3 > 0) {
    lastpointer = pointer;
    pointer--;
    return true;
  }
  return false;
}

const MenuSelectOptionContainer * MenuSection::getSectionContainer(unsigned int id) const{
  if (id <= sectioncontainers.size() - 1) {
    return &sectioncontainers[id];
  }
  return NULL;
}

unsigned int MenuSection::getLastSelectionPointer() const {
  return lastpointer;
}

unsigned int MenuSection::getSelectionPointer() const {
  return pointer;
}

std::shared_ptr<MenuSelectOptionElement> MenuSection::getElement(unsigned int i) const {
  if (i == 0) {
    return addbutton;
  }
  if (i > sectioncontainers.size() * 3) {
    return std::shared_ptr<MenuSelectOptionElement>();
  }
  int id = (i - 1) / 3;
  int internalid = (i - 1) % 3;
  return sectioncontainers[id].getOption(internalid);
}

bool MenuSection::activateSelected() {
  std::shared_ptr<MenuSelectOptionElement> msoe = getElement(pointer);
  if (msoe->getIdentifier() == "add") {
    addSection();
    needsredraw = true;
    return false;
  }
  else if (msoe->getIdentifier() == "delete") {
    sectioncontainers.erase(sectioncontainers.begin() + ((pointer - 1) / 3));
    if (!getElement(pointer)) {
      goUp();
    }
    lastpointer = pointer;
    needsredraw = true;
    return false;
  }
  return msoe->activate();
}

bool MenuSection::needsRedraw() {
  bool redraw = needsredraw;
  needsredraw = false;
  return redraw;
}

unsigned int MenuSection::getHeaderRow() const {
  return row;
}

unsigned int MenuSection::getHeaderCol() const {
  return col;
}

unsigned int MenuSection::size() const {
  return sectioncontainers.size();
}

void MenuSection::addSection(std::string nametext, const Path & path) {
  std::shared_ptr<MenuSelectOptionElement> name(std::make_shared<MenuSelectOptionTextField>("name", 0, 0, "Name:", nametext, 11, 32, false));
  std::shared_ptr<MenuSelectOptionElement> pathe(std::make_shared<MenuSelectOptionTextField>("path", 0, 0, "Path:", path.toString(), 30, 64, false));
  std::shared_ptr<MenuSelectOptionElement> del(std::make_shared<MenuSelectOptionTextButton>("delete", 0, 0, "<X>"));
  MenuSelectOptionContainer msoc = MenuSelectOptionContainer();
  msoc.addElement(name);
  msoc.addElement(pathe);
  msoc.addElement(del);
  sectioncontainers.push_back(msoc);
}
void MenuSection::addSection() {
  addSection("", "");
}

void MenuSection::enterFocusFrom(int dir) {
  focus = true;
  pointer = 0;
}
