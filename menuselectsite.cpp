#include "menuselectsite.h"

MenuSelectSite::MenuSelectSite() {
}

void MenuSelectSite::setWindow(WINDOW * window) {
  this->window = window;
  pointer = 0;
  maxheight = 0;
}

void MenuSelectSite::goNext() {
  print(pointer-1, false);
  if (pointer < sites.size()) pointer++;
  else if (sites.size() > 0) pointer = 1;
  print(pointer-1, true);
}

void MenuSelectSite::goPrev() {
  print(pointer-1, false);
  if (pointer > 0) pointer--;
  if (pointer == 0) pointer = sites.size();
  print(pointer-1, true);
}

void MenuSelectSite::add(Site * site, int row, int col) {
  SiteThread * st = global->getSiteThreadManager()->getSiteThread(site->getName());
  sites.push_back(MenuSelectSiteElement(site, st, row, col));
  if (pointer == 0) pointer = 1;
}

Site * MenuSelectSite::getSite() {
  if (sites.size() == 0) return NULL;
  return sites[pointer-1].getSite();
}

void MenuSelectSite::prepareRefill() {
  sites.clear();
}

void MenuSelectSite::print() {
  for (int i = 0; i < sites.size(); i++) {
    while (pointer > sites.size()) pointer--;
    print(i, i+1 == pointer);
  }
}

void MenuSelectSite::print(int index, bool highlight) {
  if (index < 0) return;
  std::string line = " ";
  std::string add = "";
  int linelen;
  int addlen;
  line.append(sites[index].getSite()->getName() + " ");
  add = global->int2Str(sites[index].getSiteThread()->getCurrLogins()) + "/" + global->int2Str(sites[index].getSite()->getMaxLogins());
  linelen = line.length();
  addlen = add.length();
  for (int i = 0; i < 14 - linelen - addlen; i++) line.append(" ");
  line.append(add + " ");
  add = global->int2Str(sites[index].getSiteThread()->getCurrUp()) + "/" + global->int2Str(sites[index].getSite()->getMaxUp());
  linelen = line.length();
  addlen = add.length();
  for (int i = 0; i < 23 - linelen - addlen; i++) line.append(" ");
  line.append(add + " ");
  add = global->int2Str(sites[index].getSiteThread()->getCurrDown()) + "/" + global->int2Str(sites[index].getSite()->getMaxDown());
  linelen = line.length();
  addlen = add.length();
  for (int i = 0; i < 34 - linelen - addlen; i++) line.append(" ");
  line.append(add + " ");
  if (highlight) wattron(window, A_REVERSE);
  TermInt::printStr(window, sites[index].getRow(), sites[index].getCol(), line);
  if (highlight) wattroff(window, A_REVERSE);
}
