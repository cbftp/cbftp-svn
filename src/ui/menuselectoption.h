#pragma once

#include <string>
#include <vector>

#include "focusablearea.h"

class MenuSelectOption : public FocusableArea {
  private:
    unsigned int pointer;
    unsigned int lastpointer;
    std::vector<MenuSelectOptionElement *> options;
  public:
    MenuSelectOption();
    bool goDown();
    bool goUp();
    bool goRight();
    bool goLeft();
    void addStringField(int, int, std::string, std::string, std::string, bool);
    void addStringField(int, int, std::string, std::string, std::string, bool, int);
    void addStringField(int, int, std::string, std::string, std::string, bool, int, int);
    void addIntArrow(int, int, std::string, std::string, int, int, int);
    void addCheckBox(int, int, std::string, std::string, bool);
    void addTextButton(int, int, std::string, std::string);
    void addTextButtonNoContent(int, int, std::string, std::string);
    MenuSelectOptionElement * getElement(unsigned int);
    unsigned int getLastSelectionPointer();
    unsigned int getSelectionPointer();
    bool activateSelected();
    unsigned int size();
    void enterFocusFrom(int);
    void clear();
};
