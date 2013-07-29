#include "menuselectsiteelement.h"

MenuSelectSiteElement::MenuSelectSiteElement(Site * site, SiteLogic * sitelogic, int row, int col) {
  this->site = site;
  this->sitelogic = sitelogic;
  this->row = row;
  this->col = col;
}

Site * MenuSelectSiteElement::getSite() {
  return site;
}

SiteLogic * MenuSelectSiteElement::getSiteLogic() {
  return sitelogic;
}

int MenuSelectSiteElement::getCol() {
  return col;
}

int MenuSelectSiteElement::getRow() {
  return row;
}
