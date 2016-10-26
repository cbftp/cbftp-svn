#include "menuselectsiteelement.h"

MenuSelectSiteElement::MenuSelectSiteElement(const Pointer<Site> & site, const Pointer<SiteLogic> & sitelogic, int row, int col) {
  this->site = site;
  this->sitelogic = sitelogic;
  this->row = row;
  this->col = col;
}

const Pointer<Site> & MenuSelectSiteElement::getSite() const {
  return site;
}

const Pointer<SiteLogic> & MenuSelectSiteElement::getSiteLogic() const {
  return sitelogic;
}

int MenuSelectSiteElement::getCol() const {
  return col;
}

int MenuSelectSiteElement::getRow() const {
  return row;
}
