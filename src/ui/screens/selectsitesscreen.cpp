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

void SelectSitesScreen::initialize(unsigned int row, unsigned int col, std::string preselectstr, std::string purpose, Site * skipsite) {
  sm = global->getSiteManager();
  std::vector<Site *>::const_iterator it;
  this->purpose = purpose;
  std::map<std::string, bool> preselected;
  while (true) {
    size_t commapos = preselectstr.find(",");
    if (commapos != std::string::npos) {
      preselected[preselectstr.substr(0, commapos)] = true;
      preselectstr = preselectstr.substr(commapos + 1);
    }
    else {
      preselected[preselectstr] = true;
      break;
    }
  }
  int y = 1;
  mso.clear();
  for (it = sm->getSitesIteratorBegin(); it != sm->getSitesIteratorEnd(); it++) {
    if (*it == skipsite) {
      continue;
    }
    std::string sitename = (*it)->getName();
    bool selected = preselected.find(sitename) != preselected.end();
    mso.addCheckBox(y++, 1, sitename, sitename, selected);
  }
  mso.enterFocusFrom(0);
  init(row, col);
}

void SelectSitesScreen::redraw() {
  ui->erase();
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
  }
}

void SelectSitesScreen::update() {
  MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
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
      if (mso.goUp()) {
        ui->update();
      }
      break;
    case KEY_DOWN:
      if (mso.goDown()) {
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
        MenuSelectOptionCheckBox * msocb = (MenuSelectOptionCheckBox *) mso.getElement(i);
        if (msocb->getData()) {
          blockstr += msocb->getIdentifier() + ",";
        }
      }
      blockstr = blockstr.substr(0, blockstr.length() - 1);
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
