#include "selectsitesscreen.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncheckbox.h"

#include "../../sitemanager.h"
#include "../../site.h"
#include "../../globalcontext.h"

extern GlobalContext * global;

SelectSitesScreen::SelectSitesScreen(Ui * ui) {
  this->ui = ui;
}

void SelectSitesScreen::initialize(unsigned int row, unsigned int col, std::string purpose, std::list<Site *> preselectedsites, std::list<Site *> excludedsites) {
  sm = global->getSiteManager();
  this->purpose = purpose;
  preselected.clear();
  excluded.clear();
  for (std::list<Site *>::iterator it = preselectedsites.begin(); it != preselectedsites.end(); it++) {
    preselected[*it] = true;
  }
  for (std::list<Site *>::iterator it = excludedsites.begin(); it != excludedsites.end(); it++) {
    excluded[*it] = true;
  }
  mso.clear();
  mso.enterFocusFrom(0);
  std::vector<Site *>::const_iterator it;
  for (it = sm->begin(); it != sm->end(); it++) {
    if (excluded.find(*it) != excluded.end()) {
      continue;
    }
    bool selected = preselected.find(*it) != preselected.end();
    tempsites.push_back(std::pair<std::string, bool>((*it)->getName(), selected));
  }
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
}

void SelectSitesScreen::update() {
  Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText(), true);
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
}

void SelectSitesScreen::keyPressed(unsigned int ch) {
  unsigned int pagerows = (unsigned int) row * 0.6;
  bool activation;
  switch(ch) {
    case KEY_UP:
      if (mso.goUp() || mso.goPrevious()) {
        ui->update();
      }
      break;
    case KEY_DOWN:
      if (mso.goDown() || mso.goNext()) {
        ui->update();
      }
      break;
    case KEY_LEFT:
      if (mso.goLeft()) {
        ui->update();
      }
      break;
    case KEY_RIGHT:
      if (mso.goRight()) {
        ui->update();
      }
      break;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (!mso.goDown()) {
          break;
        }
      }
      ui->redraw();
      break;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (!mso.goUp()) {
          break;
        }
      }
      ui->redraw();
      break;
    case 32:
    case 10:
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      if (!activation) {
        ui->update();
        break;
      }
      ui->update();
      ui->setLegend();
      break;
      break;
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
      break;
    }
    case 27: // esc
    case 'c':
      ui->returnToLast();
      break;
  }
}

std::string SelectSitesScreen::getLegendText() const {
  return "[d]one - [c]ancel - [Arrowkeys] Navigate";
}

std::string SelectSitesScreen::getInfoLabel() const {
  if (purpose.length()) {
    return "SELECT SITES - " + purpose;
  }
  return "SELECT SITES";
}
