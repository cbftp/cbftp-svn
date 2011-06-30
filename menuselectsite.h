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

extern GlobalContext * global;

class MenuSelectSite {
  private:
    int pointer;
    WINDOW * window;
    std::vector<MenuSelectSiteElement> sites;
  public:
    MenuSelectSite(WINDOW *);
    void goNext();
    void goPrev();
    void add(Site *, int, int);
    Site * getSite();
    void print();
    void print(int, bool);
};
