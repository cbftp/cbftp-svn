#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <ncurses.h>

#include "../sitethreadmanager.h"
#include "../sitethread.h"
#include "../site.h"
#include "../globalcontext.h"

#include "focusablearea.h"
#include "menuselectsiteelement.h"
#include "termint.h"

extern GlobalContext * global;

class MenuSelectSite : public FocusableArea {
  private:
    unsigned int pointer;
    unsigned int maxheight;
    WINDOW * window;
    std::vector<MenuSelectSiteElement> sites;
  public:
    MenuSelectSite();
    void setWindow(WINDOW *);
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
