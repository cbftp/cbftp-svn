#pragma once

#include <string>
#include <vector>

#include "focusablearea.h"

class MenuSelectOptionTextArrow;
class MenuSelectAdjustableLine;
class MenuSelectOptionTextButton;

class MenuSelectOption : public FocusableArea {
  private:
    unsigned int pointer;
    unsigned int lastpointer;
    std::vector<MenuSelectOptionElement *> options;
    std::vector<MenuSelectAdjustableLine *> adjustablelines;
  public:
    MenuSelectOption();
    bool goDown();
    bool goUp();
    bool goRight();
    bool goLeft();
    bool goNext();
    bool goPrevious();
    void addStringField(int, int, std::string, std::string, std::string, bool);
    void addStringField(int, int, std::string, std::string, std::string, bool, int);
    void addStringField(int, int, std::string, std::string, std::string, bool, int, int);
    MenuSelectOptionTextArrow * addTextArrow(int, int, std::string, std::string);
    void addIntArrow(int, int, std::string, std::string, int, int, int);
    void addCheckBox(int, int, std::string, std::string, bool);
    MenuSelectOptionTextButton * addTextButton(int, int, std::string, std::string);
    MenuSelectOptionTextButton * addTextButtonNoContent(int, int, std::string, std::string);
    MenuSelectAdjustableLine * addAdjustableLine();
    MenuSelectOptionElement * getElement(unsigned int) const;
    MenuSelectOptionElement * getElement(std::string) const;
    unsigned int getLastSelectionPointer() const;
    unsigned int getSelectionPointer() const;
    bool activateSelected();
    unsigned int size() const;
    void adjustLines(unsigned int);
    void enterFocusFrom(int);
    void clear();
    void checkPointer();
    void reset();
};
