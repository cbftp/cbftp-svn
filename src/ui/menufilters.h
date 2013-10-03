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
    void initialize(int, int, std::list<std::string>::iterator, std::list<std::string>::iterator);
    bool goDown();
    bool goUp();
    bool goRight();
    bool goLeft();
    MenuSelectOptionContainer * getSectionContainer(unsigned int);
    unsigned int getLastSelectionPointer();
    unsigned int getSelectionPointer();
    bool activateSelected();
    MenuSelectOptionElement * getElement(unsigned int);
    unsigned int getHeaderRow();
    unsigned int getHeaderCol();
    unsigned int size();
    bool needsRedraw();
    bool addButtonPressed();
    void enterFocusFrom(int);
    void addFilter();
    void clear();
};
