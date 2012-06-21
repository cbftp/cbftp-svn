#pragma once

#include <string>
#include <vector>

#include "menuselectoptionelement.h"
#include "menuselectoptiontextfield.h"
#include "menuselectoptionnumarrow.h"
#include "menuselectoptioncheckbox.h"
#include "globalcontext.h"

extern GlobalContext * global;

class MenuSelectOption {
  private:
    int pointer;
    int lastpointer;
    WINDOW * window;
    std::vector<MenuSelectOptionElement *> options;
  public:
    MenuSelectOption();
    bool goNext();
    bool goPrev();
    void addStringField(int, int, std::string, std::string, std::string, bool);
    void addIntArrow(int, int, std::string, std::string, int, int, int);
    void addCheckBox(int, int, std::string, std::string, bool);
    MenuSelectOptionElement * getElement(int);
    int getLastSelectionPointer();
    int getSelectionPointer();
    int size();
    void clear();
};
