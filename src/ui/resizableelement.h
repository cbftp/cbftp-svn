#pragma once

#include "menuselectoptionelement.h"

#define RESIZE_REMOVE 1191
#define RESIZE_WITHDOTS 1192
#define RESIZE_WITHLAST3 1193
#define RESIZE_CUTEND 1194

class ResizableElement : public MenuSelectOptionElement {
public:
  virtual ~ResizableElement() {
  }
  virtual unsigned int wantedWidth() = 0;
  unsigned int priority() {
    return prio;
  }
  unsigned int resizeMethod() {
    return resizemethod;
  }
  void setMaxWidth(unsigned int maxwidth) {
    this->maxwidth = maxwidth;
  }
  void setPriority(unsigned int prio) {
    this->prio = prio;
  }
  void setResizeMethod(unsigned int resizemethod) {
    this->resizemethod = resizemethod;
  }
  bool isVisible() {
    return visible;
  }
  void setVisible(bool visible) {
    this->visible = visible;
  }
  bool isExpandable() {
    return expandable;
  }
  void setExpandable(bool expandable) {
    this->expandable = expandable;
  }
protected:
  unsigned int maxwidth;
  unsigned int resizemethod;
private:
  unsigned int prio;
  bool visible;
  bool expandable;
};
