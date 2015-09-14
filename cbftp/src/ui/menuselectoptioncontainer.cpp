#include "menuselectoptioncontainer.h"

#include <stdlib.h>

#include "menuselectoptionelement.h"

MenuSelectOptionContainer::MenuSelectOptionContainer() {

}

MenuSelectOptionContainer::~MenuSelectOptionContainer() {

}

void MenuSelectOptionContainer::addElement(Pointer<MenuSelectOptionElement> msoe) {
  elements.push_back(msoe);
}

Pointer<MenuSelectOptionElement> MenuSelectOptionContainer::getOption(unsigned int id) const {
  if (elements.size() > 0 && elements.size() - 1 >= id) {
    return elements[id];
  }
  return Pointer<MenuSelectOptionElement>();
}
