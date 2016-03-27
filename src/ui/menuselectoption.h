#pragma once

#include <string>
#include <vector>

#include "focusablearea.h"

#include "../core/pointer.h"

class MenuSelectOptionElement;
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
    std::vector<Pointer<MenuSelectOptionElement> > options;
    std::vector<Pointer<MenuSelectAdjustableLine> > adjustablelines;
  public:
    MenuSelectOption();
    ~MenuSelectOption();
    bool goDown();
    bool goUp();
    bool goRight();
    bool goLeft();
    bool goNext();
    bool goPrevious();
    Pointer<MenuSelectOptionTextField> addStringField(int, int, std::string, std::string, std::string, bool);
    Pointer<MenuSelectOptionTextField> addStringField(int, int, std::string, std::string, std::string, bool, int);
    Pointer<MenuSelectOptionTextField> addStringField(int, int, std::string, std::string, std::string, bool, int, int);
    Pointer<MenuSelectOptionTextArrow> addTextArrow(int, int, std::string, std::string);
    Pointer<MenuSelectOptionNumArrow> addIntArrow(int, int, std::string, std::string, int, int, int);
    Pointer<MenuSelectOptionCheckBox> addCheckBox(int, int, std::string, std::string, bool);
    Pointer<MenuSelectOptionTextButton> addTextButton(int, int, std::string, std::string);
    Pointer<MenuSelectOptionTextButton> addTextButtonNoContent(int, int, std::string, std::string);
    Pointer<MenuSelectAdjustableLine> addAdjustableLine();
    Pointer<MenuSelectAdjustableLine> addAdjustableLineBefore(Pointer<MenuSelectAdjustableLine>);
    Pointer<MenuSelectOptionElement> getElement(unsigned int) const;
    Pointer<MenuSelectOptionElement> getElement(std::string) const;
    std::vector<Pointer<MenuSelectAdjustableLine> >::iterator linesBegin();
    std::vector<Pointer<MenuSelectAdjustableLine> >::iterator linesEnd();
    Pointer<MenuSelectAdjustableLine> getAdjustableLine(Pointer<MenuSelectOptionElement>) const;
    bool swapLineWithNext(Pointer<MenuSelectAdjustableLine>);
    bool swapLineWithPrevious(Pointer<MenuSelectAdjustableLine>);
    int getLineIndex(Pointer<MenuSelectAdjustableLine>);
    void removeAdjustableLine(Pointer<MenuSelectAdjustableLine>);
    void removeElement(Pointer<MenuSelectOptionElement>);
    void setPointer(Pointer<MenuSelectOptionElement>);
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
