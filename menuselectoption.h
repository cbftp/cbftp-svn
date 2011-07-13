#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <ncurses.h>

#include "menuselectoptionelement.h"
#include "globalcontext.h"

extern GlobalContext * global;

class MenuSelectOption {
  private:
    int pointer;
    int maxheight;
    WINDOW * window;
    std::vector<MenuSelectOptionElement> options;
  public:
    MenuSelectOption(WINDOW *);
    void goNext();
    void goPrev();
    void addStringField(int, int, std::string, std::string, std::string);
    void addIntArrowField(int, int, std::string, std::string, int);
    int getSelectionDataCol();
    int getSelectionDataRow();
    MenuSelectOptionElement & getSelection();
    void clear();
    void print();
    void print(int, bool);
};
