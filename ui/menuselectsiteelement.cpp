#include "menuselectsiteelement.h"

MenuSelectSiteElement::MenuSelectSiteElement(Site * site, SiteThread * sitethread, int row, int col) {
  this->site = site;
  this->sitethread = sitethread;
  this->row = row;
  this->col = col;
}

Site * MenuSelectSiteElement::getSite() {
  return site;
}

SiteThread * MenuSelectSiteElement::getSiteThread() {
  return sitethread;
}

int MenuSelectSiteElement::getCol() {
  return col;
}

int MenuSelectSiteElement::getRow() {
  return row;
}
