#include "menuselectadjustableline.h"

#include <unistd.h>

#include "resizableelement.h"

void MenuSelectAdjustableLine::addElement(Pointer<ResizableElement> re, unsigned int prio) {
  addElement(re, prio, RESIZE_REMOVE);
}

void MenuSelectAdjustableLine::addElement(Pointer<ResizableElement> re, unsigned int prio, unsigned int resizemethod) {
  addElement(re, prio, resizemethod, false);
}

void MenuSelectAdjustableLine::addElement(Pointer<ResizableElement> re, unsigned int prio, unsigned int resizemethod, bool expandable) {
  addElement(re, prio, prio, resizemethod, expandable);
}

void MenuSelectAdjustableLine::addElement(Pointer<ResizableElement> re, unsigned int highprio, unsigned int lowprio, unsigned int resizemethod, bool expandable) {
  re->setHighPriority(highprio);
  re->setLowPriority(lowprio);
  re->setResizeMethod(resizemethod);
  re->setVisible(true);
  re->setExpandable(expandable);
  elements.push_back(re);
}

Pointer<ResizableElement> MenuSelectAdjustableLine::getElement(unsigned int pos) const {
  if (!elements.size() || elements.size() - 1 < pos) {
    return Pointer<ResizableElement>();
  }
  return elements[pos];
}

unsigned int MenuSelectAdjustableLine::size() const {
  return elements.size();
}
