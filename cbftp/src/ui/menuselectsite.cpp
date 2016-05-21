#include "menuselectsite.h"

#include "../sitelogicmanager.h"
#include "../sitelogic.h"
#include "../site.h"
#include "../globalcontext.h"
#include "../util.h"

#include "termint.h"
#include "menuselectsiteelement.h"

MenuSelectSite::MenuSelectSite() {
}

MenuSelectSite::~MenuSelectSite() {

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
  SiteLogic * st = global->getSiteLogicManager()->getSiteLogic(site->getName());
  sites.push_back(makePointer<MenuSelectSiteElement>(site, st, row, col));
}

Site * MenuSelectSite::getSite() const {
  if (sites.size() == 0) return NULL;
  return sites[pointer]->getSite();
}

void MenuSelectSite::prepareRefill() {
  sites.clear();
}

Pointer<MenuSelectSiteElement> MenuSelectSite::getSiteElement(unsigned int i) {
  return sites[i];
}

unsigned int MenuSelectSite::size() const {
  return sites.size();
}

unsigned int MenuSelectSite::getSelectionPointer() const {
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

std::string MenuSelectSite::getSiteLine(unsigned int index) const {
  Site * site = sites[index]->getSite();
  SiteLogic * sitelogic = sites[index]->getSiteLogic();
  std::string line = "";
  std::string add = "";
  int linelen;
  int addlen;
  line.append(site->getName() + " ");
  add = util::int2Str(sitelogic->getCurrLogins());
  if (!site->unlimitedLogins()) {
    add += "/" + util::int2Str(site->getMaxLogins());
  }
  linelen = line.length();
  addlen = add.length();
  for (int i = 0; i < 14 - linelen - addlen; i++) line.append(" ");
  line.append(add + " ");
  add = util::int2Str(sitelogic->getCurrUp());
  if (!site->unlimitedUp()) {
    add += "/" + util::int2Str(site->getMaxUp());
  }
  linelen = line.length();
  addlen = add.length();
  for (int i = 0; i < 23 - linelen - addlen; i++) line.append(" ");
  line.append(add + " ");
  add = util::int2Str(sitelogic->getCurrDown());
  if (!site->unlimitedDown()) {
    add += "/" + util::int2Str(site->getMaxDown());
  }
  linelen = line.length();
  addlen = add.length();
  for (int i = 0; i < 34 - linelen - addlen; i++) line.append(" ");
  line.append(add);
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

void MenuSelectSite::setPointer(unsigned int newpos) {
  pointer = newpos;
  checkPointer();
}
