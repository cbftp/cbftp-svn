#include "menuselectoptioncontainer.h"

MenuSelectOptionContainer::MenuSelectOptionContainer() {

}

void MenuSelectOptionContainer::addElement(MenuSelectOptionElement * msoe) {
  elements.push_back(msoe);
}

MenuSelectOptionElement * MenuSelectOptionContainer::getOption(unsigned int id) {
  if (elements.size() > 0 && elements.size() - 1 >= id) {
    return elements[id];
  }
  return NULL;
}
