#include "menuselectsite.h"

MenuSelectSite::MenuSelectSite() {
}

void MenuSelectSite::setWindow(WINDOW * window) {
  this->window = window;
  pointer = 0;
  maxheight = 0;
}

bool MenuSelectSite::goDown() {
  if ((unsigned int) pointer + 1 < sites.size()) {
    pointer++;
    return true;
  }
  else if (leavedown) {
    focus = false;
    return true;
  }
  return false;
}

bool MenuSelectSite::goUp() {
  if (pointer > 0) {
    pointer--;
    return true;
  }
  else if (leaveup) {
    focus = false;
    return true;
  }
  return false;
}

void MenuSelectSite::add(Site * site, int row, int col) {
  SiteThread * st = global->getSiteThreadManager()->getSiteThread(site->getName());
  sites.push_back(MenuSelectSiteElement(site, st, row, col));
}

Site * MenuSelectSite::getSite() {
  if (sites.size() == 0) return NULL;
  return sites[pointer].getSite();
}

void MenuSelectSite::prepareRefill() {
  sites.clear();
}

MenuSelectSiteElement * MenuSelectSite::getSiteElement(unsigned int i) {
  return &sites[i];
}

unsigned int MenuSelectSite::size() {
  return sites.size();
}

unsigned int MenuSelectSite::getSelectionPointer() {
  return pointer;
}

void MenuSelectSite::enterFocusFrom(int dir) {
  focus = true;
  if (dir == 2) { // bottom
    pointer = size() - 1;
  }
  else {
    pointer = 0;
  }
}

std::string MenuSelectSite::getSiteLine(unsigned int index) {
  Site * site = sites[index].getSite();
  SiteThread * sitethread = sites[index].getSiteThread();
  std::string line = " ";
  std::string add = "";
  int linelen;
  int addlen;
  line.append(site->getName() + " ");
  add = global->int2Str(sitethread->getCurrLogins());
  if (!site->unlimitedLogins()) {
    add += "/" + global->int2Str(site->getMaxLogins());
  }
  linelen = line.length();
  addlen = add.length();
  for (int i = 0; i < 14 - linelen - addlen; i++) line.append(" ");
  line.append(add + " ");
  add = global->int2Str(sitethread->getCurrUp());
  if (!site->unlimitedUp()) {
    add += "/" + global->int2Str(site->getMaxUp());
  }
  linelen = line.length();
  addlen = add.length();
  for (int i = 0; i < 23 - linelen - addlen; i++) line.append(" ");
  line.append(add + " ");
  add = global->int2Str(sitethread->getCurrDown());
  if (!site->unlimitedDown()) {
    add += "/" + global->int2Str(site->getMaxDown());
  }
  linelen = line.length();
  addlen = add.length();
  for (int i = 0; i < 34 - linelen - addlen; i++) line.append(" ");
  line.append(add + " ");
  return line;
}

void MenuSelectSite::checkPointer() {
  if (pointer >= size()) {
    pointer = size() - 1;
  }
  if (size() == 0) {
    pointer = 0;
  }
}
