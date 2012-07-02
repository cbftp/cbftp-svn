#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <ncurses.h>

#include "sitethreadmanager.h"
#include "sitethread.h"
#include "site.h"
#include "menuselectsiteelement.h"
#include "globalcontext.h"
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
    void prepareRefill();
    void print();
    void print(int, bool);
};
