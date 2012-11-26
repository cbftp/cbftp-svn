#include "focusablearea.h"

FocusableArea::FocusableArea() {
  focus = false;
  leaveup = false;
  leavedown = false;
  leaveleft = false;
  leaveright = false;
}

FocusableArea::~FocusableArea() {

}

bool FocusableArea::isFocused() {
  return focus;
}

void FocusableArea::enterFocusFrom(int direction) {
  focus = true;
}

bool FocusableArea::goUp() {
  return false;
}

bool FocusableArea::goDown() {
  return false;
}

bool FocusableArea::goLeft() {
  return false;
}

bool FocusableArea::goRight() {
  return false;
}

void FocusableArea::makeLeavableUp() {
  leaveup = true;
}

void FocusableArea::makeLeavableDown() {
  leavedown = true;
}

void FocusableArea::makeLeavableLeft() {
  leaveleft = true;
}

void FocusableArea::makeLeavableRight() {
  leaveright = true;
}

unsigned int FocusableArea::getLastSelectionPointer() {
  return 0;
}

bool FocusableArea::activateSelected() {
  return false;
}

MenuSelectOptionElement * FocusableArea::getElement(unsigned int i) {
  return NULL;
}
