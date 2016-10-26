#pragma once

class Site;
class SiteLogic;

#include "../core/pointer.h"

class MenuSelectSiteElement {
  private:
    Pointer<Site> site;
    Pointer<SiteLogic> sitelogic;
    int col;
    int row;
  public:
    MenuSelectSiteElement(const Pointer<Site> &, const Pointer<SiteLogic> &, int, int);
    const Pointer<Site> & getSite() const;
    const Pointer<SiteLogic> & getSiteLogic() const;
    int getCol() const;
    int getRow() const;
};
