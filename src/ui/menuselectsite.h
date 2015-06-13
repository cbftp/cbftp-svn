#pragma once

#include <string>
#include <vector>

#include "focusablearea.h"

#include "../pointer.h"

class Site;
class MenuSelectSiteElement;

class MenuSelectSite : public FocusableArea {
  private:
    unsigned int pointer;
    unsigned int maxheight;
    std::vector<Pointer<MenuSelectSiteElement> > sites;
  public:
    MenuSelectSite();
    ~MenuSelectSite();
    bool goDown();
    bool goUp();
    void add(Site *, int, int);
    void remove(Site *);
    Site * getSite() const;
    Pointer<MenuSelectSiteElement> getSiteElement(unsigned int);
    unsigned int size() const;
    unsigned int getSelectionPointer() const;
    std::string getSiteLine(unsigned int) const;
    void enterFocusFrom(int);
    void prepareRefill();
    void checkPointer();
    void setPointer(unsigned int);
};
