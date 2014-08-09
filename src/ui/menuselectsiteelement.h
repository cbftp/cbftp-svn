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
    Site * getSite() const;
    SiteLogic * getSiteLogic() const;
    int getCol() const;
    int getRow() const;
};
