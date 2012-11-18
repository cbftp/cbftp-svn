#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <ncurses.h>

#include "../sitethreadmanager.h"
#include "../sitethread.h"
#include "../site.h"
#include "../globalcontext.h"

#include "menuselectsiteelement.h"
#include "termint.h"

extern GlobalContext * global;

class MenuSelectSite {
  private:
    int pointer;
    int maxheight;
    WINDOW * window;
    std::vector<MenuSelectSiteElement> sites;
  public:
    MenuSelectSite();
    void setWindow(WINDOW *);
    void goNext();
    void goPrev();
    void add(Site *, int, int);
    void remove(Site *);
    Site * getSite();
    MenuSelectSiteElement * getSiteElement(unsigned int);
    unsigned int size();
    unsigned int getSelectionPointer();
    std::string getSiteLine(unsigned int);
    void prepareRefill();
};
