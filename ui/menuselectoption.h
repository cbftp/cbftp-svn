#pragma once

#include <string>
#include <vector>

#include "../globalcontext.h"

#include "menuselectoptionelement.h"
#include "menuselectoptiontextfield.h"
#include "menuselectoptionnumarrow.h"
#include "menuselectoptioncheckbox.h"

extern GlobalContext * global;

class MenuSelectOption {
  private:
    unsigned int pointer;
    unsigned int lastpointer;
    WINDOW * window;
    std::vector<MenuSelectOptionElement *> options;
  public:
    MenuSelectOption();
    bool goNext();
    bool goPrev();
    void addStringField(int, int, std::string, std::string, std::string, bool);
    void addIntArrow(int, int, std::string, std::string, int, int, int);
    void addCheckBox(int, int, std::string, std::string, bool);
    MenuSelectOptionElement * getElement(unsigned int);
    unsigned int getLastSelectionPointer();
    unsigned int getSelectionPointer();
    unsigned int size();
    void clear();
};
