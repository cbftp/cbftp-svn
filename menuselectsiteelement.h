#pragma once

class Site;
class SiteThread;

class MenuSelectSiteElement {
  private:
    Site * site;
    SiteThread * sitethread;
    int col;
    int row;
  public:
    MenuSelectSiteElement(Site *, SiteThread *, int, int);
    Site * getSite();
    SiteThread * getSiteThread();
    int getCol();
    int getRow();
};
