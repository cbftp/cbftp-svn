#pragma once

#include <string>
#include <vector>

#include "focusablearea.h"

class MenuSelectSiteElement;
class Site;

class MenuSelectSite : public FocusableArea {
  private:
    unsigned int pointer;
    unsigned int maxheight;
    std::vector<MenuSelectSiteElement> sites;
  public:
    MenuSelectSite();
    bool goDown();
    bool goUp();
    void add(Site *, int, int);
    void remove(Site *);
    Site * getSite();
    MenuSelectSiteElement * getSiteElement(unsigned int);
    unsigned int size();
    unsigned int getSelectionPointer();
    std::string getSiteLine(unsigned int);
    void enterFocusFrom(int);
    void prepareRefill();
    void checkPointer();
};
