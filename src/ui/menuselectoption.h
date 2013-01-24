#pragma once

#include <string>
#include <vector>

#include "../globalcontext.h"

#include "menuselectoptionelement.h"
#include "menuselectoptiontextfield.h"
#include "menuselectoptionnumarrow.h"
#include "menuselectoptioncheckbox.h"
#include "menuselectoptiontextbutton.h"
#include "focusablearea.h"

extern GlobalContext * global;

class MenuSelectOption : public FocusableArea {
  private:
    unsigned int pointer;
    unsigned int lastpointer;
    WINDOW * window;
    std::vector<MenuSelectOptionElement *> options;
  public:
    MenuSelectOption();
    bool goDown();
    bool goUp();
    bool goRight();
    bool goLeft();
    void addStringField(int, int, std::string, std::string, std::string, bool);
    void addStringField(int, int, std::string, std::string, std::string, bool, int);
    void addIntArrow(int, int, std::string, std::string, int, int, int);
    void addCheckBox(int, int, std::string, std::string, bool);
    void addTextButton(int, int, std::string, std::string);
    MenuSelectOptionElement * getElement(unsigned int);
    unsigned int getLastSelectionPointer();
    unsigned int getSelectionPointer();
    bool activateSelected();
    unsigned int size();
    void enterFocusFrom(int);
    void clear();
};
