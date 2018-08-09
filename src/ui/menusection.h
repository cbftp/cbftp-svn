#pragma once

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "focusablearea.h"
#include "menuselectoptioncontainer.h"

class MenuSelectOptionElement;
class Path;

class MenuSection : public FocusableArea {
  private:
    unsigned int row;
    unsigned int col;
    unsigned int pointer;
    unsigned int lastpointer;
    bool needsredraw;
    std::shared_ptr<MenuSelectOptionElement> addbutton;
    std::vector<MenuSelectOptionContainer> sectioncontainers;
  public:
    MenuSection();
    ~MenuSection();
    void initialize(int, int, std::map<std::string, Path>::const_iterator, std::map<std::string, Path>::const_iterator);
    bool goDown();
    bool goUp();
    bool goRight();
    bool goLeft();
    const MenuSelectOptionContainer * getSectionContainer(unsigned int) const;
    unsigned int getLastSelectionPointer() const;
    unsigned int getSelectionPointer() const;
    bool activateSelected();
    std::shared_ptr<MenuSelectOptionElement> getElement(unsigned int) const;
    unsigned int getHeaderRow() const;
    unsigned int getHeaderCol() const;
    unsigned int size() const;
    bool needsRedraw();
    void enterFocusFrom(int);
    void addSection();
    void addSection(std::string, const Path &);
    void clear();
};
