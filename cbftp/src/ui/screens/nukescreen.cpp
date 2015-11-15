#include "nukescreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../engine.h"
#include "../../sitelogic.h"
#include "../../filelist.h"
#include "../../sitelogicmanager.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptiontextbutton.h"

extern GlobalContext * global;

NukeScreen::NukeScreen(Ui * ui) {
  this->ui = ui;
}

NukeScreen::~NukeScreen() {

}

void NukeScreen::initialize(unsigned int row, unsigned int col, std::string sitestr, std::string release, FileList * filelist) {
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [n]uke - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  this->sitestr = sitestr;
  this->release = release;
  sitelogic = global->getSiteLogicManager()->getSiteLogic(sitestr);
  path = filelist->getPath();
  std::list<std::string> sections = global->getSiteManager()->getSite(sitestr)->getSectionsForPath(path);
  mso.clear();
  mso.addIntArrow(5, 1, "multiplier", "Multiplier:", 1, 1, 100);
  mso.addStringField(6, 1, "reason", "Reason:", "", false, col - 3, 512);
  mso.addTextButtonNoContent(8, 1, "nuke", "Nuke");
  mso.addTextButtonNoContent(8, 10, "cancel", "Cancel");
  mso.enterFocusFrom(0);
  init(row, col);
}

void NukeScreen::redraw() {
  ui->erase();
  ui->printStr(1, 1, "Site: " + sitestr);
  ui->printStr(2, 1, "Release: " + release);
  ui->printStr(3, 1, "Path: " + path);
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void NukeScreen::update() {
  Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    ui->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

bool NukeScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      if (mso.goUp()) {
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (mso.goDown()) {
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
    case 10:
      activeelement = mso.getElement(mso.getSelectionPointer());
      activation = activeelement->activate();
      if (!activation) {
        if (activeelement->getIdentifier() == "nuke") {
          int reqid = nuke();
          ui->returnNuke(reqid);
          return true;
        }
        else if (activeelement->getIdentifier() == "cancel") {
          ui->returnToLast();
          return true;
        }
      }
      active = true;
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      return true;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
    case 'n':
      int reqid = nuke();
      ui->returnNuke(reqid);
      return true;
  }
  return false;
}

int NukeScreen::nuke() {
  int multiplier = 1;
  std::string reason;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = mso.getElement(i);
    std::string identifier = msoe->getIdentifier();
    if (identifier == "multiplier") {
      multiplier = msoe.get<MenuSelectOptionNumArrow>()->getData();
    }
    else if (identifier == "reason") {
      reason = msoe.get<MenuSelectOptionTextField>()->getData();
    }
  }
  return sitelogic->requestNuke(path + "/" + release, multiplier, reason);
}
std::string NukeScreen::getLegendText() const {
  return currentlegendtext;
}

std::string NukeScreen::getInfoLabel() const {
  return "NUKE";
}
