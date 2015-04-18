#pragma once

#include <vector>

#include "../pointer.h"

#include "menuselectoptionelement.h"

class MenuSelectOptionContainer {
private:
  std::vector<Pointer<MenuSelectOptionElement> > elements;
public:
  MenuSelectOptionContainer();
  void addElement(Pointer<MenuSelectOptionElement>);
  Pointer<MenuSelectOptionElement> getOption(unsigned int) const;
};
