#pragma once

#include <string>
#include <list>
#include <vector>

#include "focusablearea.h"

class MenuSelectOptionContainer;
class MenuSelectOptionElement;

class MenuFilters : public FocusableArea {
  private:
    unsigned int row;
    unsigned int col;
    unsigned int pointer;
    unsigned int lastpointer;
    bool needsredraw;
    MenuSelectOptionElement * addbutton;
    std::vector<MenuSelectOptionContainer> filtercontainers;
    void addFilter(std::string);
  public:
    MenuFilters();
    void initialize(int, int, std::list<std::string>::const_iterator, std::list<std::string>::const_iterator);
    bool goDown();
    bool goUp();
    bool goRight();
    bool goLeft();
    MenuSelectOptionContainer * getSectionContainer(unsigned int);
    unsigned int getLastSelectionPointer() const;
    unsigned int getSelectionPointer() const;
    bool activateSelected();
    MenuSelectOptionElement * getElement(unsigned int) const;
    unsigned int getHeaderRow() const;
    unsigned int getHeaderCol() const;
    unsigned int size() const;
    bool needsRedraw();
    void enterFocusFrom(int);
    void addFilter();
    void clear();
};
