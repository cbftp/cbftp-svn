#include "menuselectoptioncontainer.h"

#include <stdlib.h>

MenuSelectOptionContainer::MenuSelectOptionContainer() {

}

void MenuSelectOptionContainer::addElement(MenuSelectOptionElement * msoe) {
  elements.push_back(msoe);
}

MenuSelectOptionElement * MenuSelectOptionContainer::getOption(unsigned int id) const {
  if (elements.size() > 0 && elements.size() - 1 >= id) {
    return elements[id];
  }
  return NULL;
}
