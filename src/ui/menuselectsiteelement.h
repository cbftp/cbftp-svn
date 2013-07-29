#pragma once

class Site;
class SiteLogic;

class MenuSelectSiteElement {
  private:
    Site * site;
    SiteLogic * sitelogic;
    int col;
    int row;
  public:
    MenuSelectSiteElement(Site *, SiteLogic *, int, int);
    Site * getSite();
    SiteLogic * getSiteLogic();
    int getCol();
    int getRow();
};
