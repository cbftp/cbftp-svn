#include "selectsitesscreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncheckbox.h"

#include "../../sitemanager.h"
#include "../../site.h"
#include "../../globalcontext.h"

SelectSitesScreen::SelectSitesScreen(Ui * ui) {
  this->ui = ui;
}

void SelectSitesScreen::initialize(unsigned int row, unsigned int col, std::string purpose, std::list<Pointer<Site> > preselectedsites, std::list<Pointer<Site> > excludedsites) {
  sm = global->getSiteManager();
  this->purpose = purpose;
  preselected.clear();
  excluded.clear();
  for (std::list<Pointer<Site> >::iterator it = preselectedsites.begin(); it != preselectedsites.end(); it++) {
    preselected.insert(*it);
  }
  for (std::list<Pointer<Site> >::iterator it = excludedsites.begin(); it != excludedsites.end(); it++) {
    excluded.insert(*it);
  }
  mso.clear();
  mso.enterFocusFrom(0);
  std::vector<Pointer<Site> >::const_iterator it;
  for (it = sm->begin(); it != sm->end(); it++) {
    if (excluded.find(*it) != excluded.end()) {
      continue;
    }
    bool selected = preselected.find(*it) != preselected.end();
    tempsites.push_back(std::pair<std::string, bool>((*it)->getName(), selected));
  }
  togglestate = false;
  init(row, col);
}

void SelectSitesScreen::redraw() {
  ui->erase();
  unsigned int y = 1;
  unsigned int x = 1;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionCheckBox> msocb = mso.getElement(i);
    tempsites.push_back(std::pair<std::string, bool>(msocb->getIdentifier(), msocb->getData()));
  }
  mso.clear();
  unsigned int longestsitenameinline = 0;
  std::list<std::pair<std::string, bool> >::iterator it;
  for (it = tempsites.begin(); it != tempsites.end(); it++) {
    std::string sitename = it->first;
    if (sitename.length() > longestsitenameinline) {
      longestsitenameinline = sitename.length();
    }
    if (y >= row - 1) {
      y = 1;
      x += longestsitenameinline + 7;
      longestsitenameinline = 0;
    }
    mso.addCheckBox(y++, x, sitename, sitename, it->second);
  }
  tempsites.clear();
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
  }
  if (!mso.size()) {
    ui->printStr(1, 1, "(no sites available)");
  }
}

void SelectSitesScreen::update() {
  Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText(), true);
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
}

bool SelectSitesScreen::keyPressed(unsigned int ch) {
  unsigned int pagerows = (unsigned int) row * 0.6;
  bool activation;
  switch(ch) {
    case KEY_UP:
      if (mso.goUp() || mso.goPrevious()) {
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (mso.goDown() || mso.goNext()) {
        ui->update();
      }
      return true;
    case KEY_LEFT:
      if (mso.goLeft()) {
        ui->update();
      }
      return true;
    case KEY_RIGHT:
      if (mso.goRight()) {
        ui->update();
      }
      return true;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (!mso.goDown()) {
          break;
        }
      }
      ui->redraw();
      return true;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (!mso.goUp()) {
          break;
        }
      }
      ui->redraw();
      return true;
    case 32:
    case 10: {
      Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      if (!!msoe) {
        activation = msoe->activate();
        if (!activation) {
          ui->update();
          return true;
        }
        ui->update();
        ui->setLegend();
      }
      return true;
    }
    case 'd': {
      std::string blockstr = "";
      for (unsigned int i = 0; i < mso.size(); i++) {
        Pointer<MenuSelectOptionCheckBox> msocb = mso.getElement(i);
        if (msocb->getData()) {
          blockstr += msocb->getIdentifier() + ",";
        }
      }
      if (blockstr.length() > 0) {
        blockstr = blockstr.substr(0, blockstr.length() - 1);
      }
      ui->returnSelectSites(blockstr);
      return true;
    }
    case 't': {
      bool triggered = false;
      while (!triggered && mso.size()) {
        for (unsigned int i = 0; i < mso.size(); i++) {
          Pointer<MenuSelectOptionCheckBox> msocb = mso.getElement(i);
          if (togglestate == msocb->getData()) {
            msocb->activate();
            triggered = true;
          }
        }
        togglestate = !togglestate;
      }
      ui->redraw();
      break;
    }
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string SelectSitesScreen::getLegendText() const {
  return "[d]one - [c]ancel - [Arrowkeys] Navigate - [t]oggle all";
}

std::string SelectSitesScreen::getInfoLabel() const {
  if (purpose.length()) {
    return "SELECT SITES - " + purpose;
  }
  return "SELECT SITES";
}
