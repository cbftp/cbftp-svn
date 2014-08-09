#pragma once

#include <vector>

#define RESIZE_SPACING 2

class ResizableElement;

class MenuSelectAdjustableLine {
public:
  void addElement(ResizableElement *, unsigned int);
  void addElement(ResizableElement *, unsigned int, unsigned int);
  void addElement(ResizableElement *, unsigned int, unsigned int, bool);
  ResizableElement * getElement(unsigned int) const;
  unsigned int size() const;
private:
  std::vector<ResizableElement *> elements;
};
