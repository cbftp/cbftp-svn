#pragma once

#include <string>
#include <vector>

#include "focusablearea.h"

class MenuSelectOptionTextArrow;
class MenuSelectAdjustableLine;
class MenuSelectOptionTextButton;
class MenuSelectOptionTextField;
class MenuSelectOptionNumArrow;
class MenuSelectOptionCheckBox;

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
    MenuSelectOptionTextField * addStringField(int, int, std::string, std::string, std::string, bool);
    MenuSelectOptionTextField * addStringField(int, int, std::string, std::string, std::string, bool, int);
    MenuSelectOptionTextField * addStringField(int, int, std::string, std::string, std::string, bool, int, int);
    MenuSelectOptionTextArrow * addTextArrow(int, int, std::string, std::string);
    MenuSelectOptionNumArrow * addIntArrow(int, int, std::string, std::string, int, int, int);
    MenuSelectOptionCheckBox * addCheckBox(int, int, std::string, std::string, bool);
    MenuSelectOptionTextButton * addTextButton(int, int, std::string, std::string);
    MenuSelectOptionTextButton * addTextButtonNoContent(int, int, std::string, std::string);
    MenuSelectAdjustableLine * addAdjustableLine();
    MenuSelectAdjustableLine * addAdjustableLineBefore(MenuSelectAdjustableLine *);
    MenuSelectOptionElement * getElement(unsigned int) const;
    MenuSelectOptionElement * getElement(std::string) const;
    std::vector<MenuSelectAdjustableLine *>::iterator linesBegin();
    std::vector<MenuSelectAdjustableLine *>::iterator linesEnd();
    MenuSelectAdjustableLine * getAdjustableLine(MenuSelectOptionElement *) const;
    bool swapLineWithNext(MenuSelectAdjustableLine *);
    bool swapLineWithPrevious(MenuSelectAdjustableLine *);
    int getLineIndex(MenuSelectAdjustableLine *);
    void removeAdjustableLine(MenuSelectAdjustableLine *);
    void removeElement(MenuSelectOptionElement *);
    void setPointer(MenuSelectOptionElement *);
    unsigned int getLastSelectionPointer() const;
    unsigned int getSelectionPointer() const;
    bool activateSelected();
    unsigned int size() const;
    unsigned int linesSize() const;
    void adjustLines(unsigned int);
    void enterFocusFrom(int);
    void clear();
    void checkPointer();
    void reset();
};
