#pragma once

#include <string>
#include <list>

#include "../globalcontext.h"

#include "menuselectoptioncontainer.h"
#include "menuselectoptionelement.h"
#include "menuselectoptiontextfield.h"
#include "menuselectoptiontextbutton.h"
#include "focusablearea.h"

extern GlobalContext * global;

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
