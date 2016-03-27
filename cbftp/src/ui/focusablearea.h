#pragma once

#include "../core/pointer.h"

class MenuSelectOptionElement;

class FocusableArea {
protected:
  bool focus;
  bool leaveup;
  bool leavedown;
  bool leaveright;
  bool leaveleft;
public:
  FocusableArea();
  virtual ~FocusableArea();
  virtual bool goUp();
  virtual bool goDown();
  virtual bool goLeft();
  virtual bool goRight();
  virtual bool goNext();
  virtual bool goPrevious();
  virtual void enterFocusFrom(int);
  bool isFocused() const;
  void makeLeavableUp(bool);
  void makeLeavableDown(bool);
  void makeLeavableLeft(bool);
  void makeLeavableRight(bool);
  void makeLeavableUp();
  void makeLeavableDown();
  void makeLeavableLeft();
  void makeLeavableRight();
  virtual bool activateSelected();
  virtual unsigned int getLastSelectionPointer() const;
  virtual unsigned int getSelectionPointer() const = 0;
  virtual Pointer<MenuSelectOptionElement> getElement(unsigned int) const;
};
