#include "selectsitesscreen.h"

#include "../uicommunicator.h"
#include "../termint.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptioncheckbox.h"

#include "../../sitemanager.h"
#include "../../site.h"
#include "../../globalcontext.h"

extern GlobalContext * global;

SelectSitesScreen::SelectSitesScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  sm = global->getSiteManager();
  std::vector<Site *>::iterator it;
  std::string preselectstr = uicommunicator->getArg1();
  purpose = uicommunicator->getArg2();
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
  Site * skipsite = (Site *) uicommunicator->getPointerArg();
  int y = 1;
  for (it = sm->getSitesIteratorBegin(); it != sm->getSitesIteratorEnd(); it++) {
    if (*it == skipsite) {
      continue;
    }
    std::string sitename = (*it)->getName();
    bool selected = preselected.find(sitename) != preselected.end();
    mso.addCheckBox(y++, 1, sitename, sitename, selected);
  }
  mso.enterFocusFrom(0);
  init(window, row, col);
}

void SelectSitesScreen::redraw() {
  werase(window);
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
    if (highlight) wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getContentText());
  }
}

void SelectSitesScreen::update() {
  MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  wattron(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
  wattroff(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getContentText());
}

void SelectSitesScreen::keyPressed(unsigned int ch) {
  unsigned int pagerows = (unsigned int) row * 0.6;
  bool activation;
  switch(ch) {
    case KEY_UP:
      if (mso.goUp()) {
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_DOWN:
      if (mso.goDown()) {
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_LEFT:
      if (mso.goLeft()) {
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_RIGHT:
      if (mso.goRight()) {
        uicommunicator->newCommand("update");
      }
      break;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (!mso.goDown()) {
          break;
        }
      }
      uicommunicator->newCommand("redraw");
      break;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (!mso.goUp()) {
          break;
        }
      }
      uicommunicator->newCommand("redraw");
      break;
    case 32:
    case 10:
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      if (!activation) {
        uicommunicator->newCommand("update");
        break;
      }
      uicommunicator->newCommand("updatesetlegend");
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
      uicommunicator->newCommand("returnselectsites", blockstr);
      break;
    }
    case 27: // esc
    case 'c':
      uicommunicator->newCommand("return");
      break;
  }
}

std::string SelectSitesScreen::getLegendText() {
  return "[d]one - [c]ancel - [Arrowkeys] Navigate";
}

std::string SelectSitesScreen::getInfoLabel() {
  if (purpose.length()) {
    return "SELECT SITES - " + purpose;
  }
  return "SELECT SITES";
}
