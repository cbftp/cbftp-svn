#include "nukescreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../engine.h"
#include "../../sitelogic.h"
#include "../../filelist.h"
#include "../../sitelogicmanager.h"

#include "../uicommunicator.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptionelement.h"
#include "../focusablearea.h"
#include "../termint.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextfield.h"

extern GlobalContext * global;

NukeScreen::NukeScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [n]uke - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  sitestr = uicommunicator->getArg1();
  release = uicommunicator->getArg2();
  FileList * filelist = (FileList *) uicommunicator->getPointerArg();
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitestr);
  path = filelist->getPath();
  std::list<std::string> sections = global->getSiteManager()->getSite(sitestr)->getSectionsForPath(path);
  mso.addIntArrow(5, 1, "multiplier", "Multiplier:", 1, 1, 100);
  mso.addStringField(6, 1, "reason", "Reason:", "", false, col - 3, 512);
  mso.addTextButtonNoContent(8, 1, "nuke", "Nuke");
  mso.addTextButtonNoContent(8, 10, "cancel", "Cancel");
  mso.enterFocusFrom(0);
  init(window, row, col);
}

void NukeScreen::redraw() {
  werase(window);
  std::vector<Site *>::iterator it;
  TermInt::printStr(window, 1, 1, "Site: " + sitestr);
  TermInt::printStr(window, 2, 1, "Release: " + release);
  TermInt::printStr(window, 3, 1, "Path: " + path);
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    if (highlight) wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void NukeScreen::update() {
  MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  wattron(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  wattroff(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  if (active && msoe->cursorPosition() >= 0) {
    curs_set(1);
    TermInt::moveCursor(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void NukeScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      uicommunicator->newCommand("updatesetlegend");
      return;
    }
    activeelement->inputChar(ch);
    uicommunicator->newCommand("update");
    return;
  }
  bool activation;
  std::list<std::string> sites;
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
    case 10:
      activeelement = mso.getElement(mso.getSelectionPointer());
      activation = activeelement->activate();
      if (!activation) {
        if (activeelement->getIdentifier() == "nuke") {
          int reqid = nuke();
          uicommunicator->newCommand("returnnuke", global->int2Str(reqid));
          break;
        }
        else if (activeelement->getIdentifier() == "cancel") {
          uicommunicator->newCommand("return");
          break;
        }
      }
      active = true;
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 27: // esc
    case 'c':
      uicommunicator->newCommand("return");
      break;
    case 'n':
      int reqid = nuke();
      uicommunicator->newCommand("returnnuke", global->int2Str(reqid));
      break;
  }
}

int NukeScreen::nuke() {
  int multiplier;
  std::string reason;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    std::string identifier = msoe->getIdentifier();
    if (identifier == "multiplier") {
      multiplier = ((MenuSelectOptionNumArrow *)msoe)->getData();
    }
    else if (identifier == "reason") {
      reason = ((MenuSelectOptionTextField *)msoe)->getData();
    }
  }
  return sitelogic->requestNuke(path + "/" + release, multiplier, reason);
}
std::string NukeScreen::getLegendText() {
  return currentlegendtext;
}

std::string NukeScreen::getInfoLabel() {
  return "NUKE";
}
