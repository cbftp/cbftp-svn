#include "menuselectsiteelement.h"

MenuSelectSiteElement::MenuSelectSiteElement(Site * site, SiteLogic * sitelogic, int row, int col) {
  this->site = site;
  this->sitelogic = sitelogic;
  this->row = row;
  this->col = col;
}

Site * MenuSelectSiteElement::getSite() const {
  return site;
}

SiteLogic * MenuSelectSiteElement::getSiteLogic() const {
  return sitelogic;
}

int MenuSelectSiteElement::getCol() const {
  return col;
}

int MenuSelectSiteElement::getRow() const {
  return row;
}
