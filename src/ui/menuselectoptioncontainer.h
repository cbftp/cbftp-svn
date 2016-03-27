#pragma once

#include <vector>

#include "../core/pointer.h"

class MenuSelectOptionElement;

class MenuSelectOptionContainer {
private:
  std::vector<Pointer<MenuSelectOptionElement> > elements;
public:
  MenuSelectOptionContainer();
  ~MenuSelectOptionContainer();
  void addElement(Pointer<MenuSelectOptionElement>);
  Pointer<MenuSelectOptionElement> getOption(unsigned int) const;
};
