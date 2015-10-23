#include "newracescreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../engine.h"

#include "../ui.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptionelement.h"
#include "../focusablearea.h"

extern GlobalContext * global;

NewRaceScreen::NewRaceScreen(Ui * ui) {
  this->ui = ui;
}

NewRaceScreen::~NewRaceScreen() {

}

void NewRaceScreen::initialize(unsigned int row, unsigned int col, std::string site, std::string section, std::string release) {
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [t]oggle all - [s]tart race - [S]tart race and return to browsing - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  toggleall = false;
  unsigned int y = 2;
  unsigned int x = 1;
  sectionupdate = false;
  infotext = "";
  std::string sectionstring = section;
  size_t splitpos;
  bool sectionset = false;
  int sectx = x + std::string("Section: ").length();
  msos.reset();
  while (sectionstring.length() > 0) {
    splitpos = sectionstring.find(";");
    std::string section;
    if (splitpos != std::string::npos) {
      section = sectionstring.substr(0, splitpos);
      sectionstring = sectionstring.substr(splitpos + 1);
    }
    else {
      section = sectionstring;
      sectionstring = "";
    }
    std::string buttontext = " " + section + " ";
    if (!sectionset) {
      this->section = section;
      sectionset = true;
    }
    msos.addTextButton(y, sectx, section, buttontext);
    sectx = sectx + buttontext.length();
  }
  y = y + 2;
  this->release = release;
  startsite = global->getSiteManager()->getSite(site);
  focusedarea = &msos;
  msos.makeLeavableDown();
  mso.reset();
  mso.makeLeavableUp();
  msos.enterFocusFrom(0);
  populateSiteList();
  init(row, col);
}

void NewRaceScreen::populateSiteList() {
  std::vector<Site *>::const_iterator it;
  mso.clear();
  if (!tempsites.size()) {
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      Site * site = *it;
      if (site->hasSection(section) && (site->getAllowDownload() || site->getAllowUpload())) {
        tempsites.push_back(std::pair<std::string, bool>(site->getName(), toggleall || site == startsite));
      }
    }
  }
  unsigned int y = 6;
  unsigned int x = 1;
  unsigned int longestsitenameinline = 0;
  std::list<std::pair<std::string, bool> >::iterator it2;
  for (it2 = tempsites.begin(); it2 != tempsites.end(); it2++) {
    if (it2->first.length() > longestsitenameinline) {
      longestsitenameinline = it2->first.length();
    }
    if (y >= row - 1) {
      y = 6;
      x += longestsitenameinline + 7;
      longestsitenameinline = 0;
    }
    mso.addCheckBox(y++, x, it2->first, it2->first, it2->second);
  }
  tempsites.clear();
}

void NewRaceScreen::redraw() {
  ui->erase();
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionCheckBox> msocb = mso.getElement(i);
    tempsites.push_back(std::pair<std::string, bool>(msocb->getIdentifier(), msocb->getData()));
  }
  populateSiteList();
  ui->printStr(1, 1, "Release: " + release);
  ui->printStr(2, 1, "Section: ");
  bool highlight;
  for (unsigned int i = 0; i < msos.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = msos.getElement(i);
    highlight = false;
    if (msos.isFocused() && msos.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe), highlight);
  }
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

void NewRaceScreen::update() {
  if (sectionupdate) {
    sectionupdate = false;
    tempsites.clear();
    mso.clear();
    redraw();
    return;
  }
  if (defocusedarea != NULL) {
    if (defocusedarea == &mso) {
      Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
    }
    else if (defocusedarea == &msos){
      Pointer<MenuSelectOptionElement> msoe = msos.getElement(msos.getLastSelectionPointer());
      ui->printStr(msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe));
    }
  }
  if (focusedarea == &mso) {
    Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText());
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
    msoe = mso.getElement(mso.getSelectionPointer());
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1, msoe->getLabelText(), true);
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getContentText());
    if (active && msoe->cursorPosition() >= 0) {
      ui->showCursor();
      ui->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getContentText().length() + 1 + msoe->cursorPosition());
    }
    else {
      ui->hideCursor();
    }
  }
  else {
    Pointer<MenuSelectOptionElement> msoe = msos.getElement(msos.getLastSelectionPointer());
    ui->printStr(msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe));
    msoe = msos.getElement(msos.getSelectionPointer());
    ui->printStr(msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe), true);
  }
}

void NewRaceScreen::keyPressed(unsigned int ch) {
  infotext = "";
  unsigned int pagerows = (unsigned int) (row - 6) * 0.6;
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->update();
      ui->setLegend();
      return;
    }
    activeelement->inputChar(ch);
    ui->update();
    return;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      if (focusedarea->goUp() || focusedarea->goPrevious()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &msos;
          focusedarea->enterFocusFrom(2);
        }
        ui->update();
      }
      break;
    case KEY_DOWN:
      if (focusedarea->goDown() || focusedarea->goNext()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mso;
          focusedarea->enterFocusFrom(0);
        }
        ui->update();
      }
      break;
    case KEY_LEFT:
      if (focusedarea->goLeft()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
      }
      break;
    case KEY_RIGHT:
      if (focusedarea->goRight()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
      }
      break;
    case KEY_NPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (focusedarea->goDown()) {
          if (!focusedarea->isFocused()) {
            defocusedarea = focusedarea;
            focusedarea = &mso;
            focusedarea->enterFocusFrom(0);
          }
        }
      }
      ui->redraw();
      break;
    case KEY_PPAGE:
      for (unsigned int i = 0; i < pagerows; i++) {
        if (focusedarea->goUp()) {
          if (!focusedarea->isFocused()) {
            defocusedarea = focusedarea;
            focusedarea = &msos;
            focusedarea->enterFocusFrom(2);
          }
        }
      }
      ui->redraw();
      break;
    case 10:

      activation = focusedarea->getElement(focusedarea->getSelectionPointer())->activate();
      if (!activation) {
        if (focusedarea == &msos) {
          section = msos.getElement(msos.getSelectionPointer())->getIdentifier();
          sectionupdate = true;
        }
        ui->update();
        break;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      break;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      break;
    case 's':
      if (startRace()) {
        ui->returnRaceStatus(global->getEngine()->getLatestId());
      }
      break;
    case 'S':
      if (startRace()) {
        ui->returnToLast();
      }
      break;
    case 't':
      if (!toggleall) {
        toggleall = true;
      }
      else {
        toggleall = false;
      }
      populateSiteList();
      ui->redraw();
      break;
  }
}

std::string NewRaceScreen::getLegendText() const {
  return currentlegendtext;
}

std::string NewRaceScreen::getInfoLabel() const {
  return "START NEW RACE";
}

std::string NewRaceScreen::getInfoText() const {
  return infotext;
}

std::string NewRaceScreen::getSectionButtonText(Pointer<MenuSelectOptionElement> msoe) const {
  std::string buttontext = msoe->getLabelText();
  if (msoe->getIdentifier() == section) {
    buttontext[0] = '[';
    buttontext[buttontext.length()-1] = ']';
  }
  return buttontext;
}

bool NewRaceScreen::startRace() {
  std::list<std::string> sites;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionCheckBox> msocb = mso.getElement(i);
    if (msocb->getData()) {
      sites.push_back(msocb->getIdentifier());
    }
  }
  if (sites.size() < 2) {
    infotext = "Cannot start race with less than 2 sites!";
    ui->update();
    return false;
  }
  return global->getEngine()->newRace(release, section, sites);
}
