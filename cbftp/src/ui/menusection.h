#pragma once

#include <string>
#include <vector>
#include <map>

#include "../core/pointer.h"

#include "focusablearea.h"
#include "menuselectoptioncontainer.h"

class MenuSelectOptionElement;

class MenuSection : public FocusableArea {
  private:
    unsigned int row;
    unsigned int col;
    unsigned int pointer;
    unsigned int lastpointer;
    bool needsredraw;
    Pointer<MenuSelectOptionElement> addbutton;
    std::vector<MenuSelectOptionContainer> sectioncontainers;
  public:
    MenuSection();
    ~MenuSection();
    void initialize(int, int, std::map<std::string, std::string>::const_iterator, std::map<std::string, std::string>::const_iterator);
    bool goDown();
    bool goUp();
    bool goRight();
    bool goLeft();
    const MenuSelectOptionContainer * getSectionContainer(unsigned int) const;
    unsigned int getLastSelectionPointer() const;
    unsigned int getSelectionPointer() const;
    bool activateSelected();
    Pointer<MenuSelectOptionElement> getElement(unsigned int) const;
    unsigned int getHeaderRow() const;
    unsigned int getHeaderCol() const;
    unsigned int size() const;
    bool needsRedraw();
    void enterFocusFrom(int);
    void addSection();
    void addSection(std::string, std::string);
    void clear();
};
