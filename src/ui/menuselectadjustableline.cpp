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

std::pair<unsigned int, unsigned int> MenuSelectAdjustableLine::getMinMaxCol() const {
  if (elements.empty()) {
    return std::pair<unsigned int, unsigned int>(0, 0);
  }
  unsigned int min = elements[0]->getCol();
  const Pointer<ResizableElement> & maxelem = elements[elements.size() - 1];
  unsigned int max = maxelem->getCol() + maxelem->getLabelText().length() - 1;
  return std::pair<unsigned int, unsigned int>(min, max);
}

unsigned int MenuSelectAdjustableLine::size() const {
  return elements.size();
}

bool MenuSelectAdjustableLine::empty() const {
  return elements.empty();
}
