#pragma once

#include <string>
#include <vector>

class MenuSelectOptionElement;

class MenuSelectOptionContainer {
private:
  std::vector<MenuSelectOptionElement *> elements;
public:
  MenuSelectOptionContainer();
  void addElement(MenuSelectOptionElement *);
  MenuSelectOptionElement * getOption(unsigned int);
};
