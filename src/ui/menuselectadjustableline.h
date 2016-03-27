#pragma once

#include <vector>

#include "../core/pointer.h"

#define RESIZE_SPACING 2
#define RESIZE_SPACING_SHORT 1

class ResizableElement;

class MenuSelectAdjustableLine {
public:
  void addElement(Pointer<ResizableElement>, unsigned int);
  void addElement(Pointer<ResizableElement>, unsigned int, unsigned int);
  void addElement(Pointer<ResizableElement>, unsigned int, unsigned int, bool);
  Pointer<ResizableElement> getElement(unsigned int) const;
  unsigned int size() const;
private:
  std::vector<Pointer<ResizableElement> > elements;
};
