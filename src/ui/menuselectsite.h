#pragma once

#include <string>
#include <vector>

#include "focusablearea.h"

#include "../core/pointer.h"

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
    void add(const Pointer<Site> &, int, int);
    void remove(const Pointer<Site> &);
    const Pointer<Site> getSite() const;
    const Pointer<MenuSelectSiteElement> & getSiteElement(unsigned int);
    unsigned int size() const;
    unsigned int getSelectionPointer() const;
    std::string getSiteLine(unsigned int) const;
    void enterFocusFrom(int);
    void prepareRefill();
    void checkPointer();
    void setPointer(unsigned int);
};
