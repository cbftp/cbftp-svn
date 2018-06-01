#include "newracescreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../engine.h"
#include "../../race.h"

#include "../ui.h"
#include "../menuselectoptioncheckbox.h"
#include "../menuselectoptiontextbutton.h"
#include "../menuselectoptiontextarrow.h"
#include "../menuselectoptionelement.h"
#include "../focusablearea.h"

NewRaceScreen::NewRaceScreen(Ui * ui) {
  this->ui = ui;
}

NewRaceScreen::~NewRaceScreen() {

}

void NewRaceScreen::initialize(unsigned int row, unsigned int col, const std::string & site, const std::string & section, const std::string & release) {
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [t]oggle all - [s]tart spread job - [S]tart spread job and return to browsing - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  toggleall = false;
  unsigned int y = 2;
  unsigned int x = 1;
  infotext = "";
  std::string sectionstring = section;
  size_t splitpos;
  bool sectionset = false;
  int sectx = x + std::string("Section: ").length();
  msos.reset();
  msota = msos.addTextArrow(y++, 1, "profile", "Profile:");
  msota->addOption("Race", SPREAD_RACE);
  msota->addOption("Distribute", SPREAD_DISTRIBUTE);
  msota->setId(1);
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
    Pointer<MenuSelectOptionTextButton> msotb = msos.addTextButton(y, sectx, section, buttontext);
    msotb->setId(0);
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
  std::vector<Pointer<Site> >::const_iterator it;
  mso.clear();
  if (!tempsites.size()) {
    for (it = global->getSiteManager()->begin(); it != global->getSiteManager()->end(); it++) {
      const Pointer<Site> & site = *it;
      if (site->hasSection(section) && !site->getDisabled() && site->getSkipList().check((site->getSectionPath(section) / release).toString(), true, false).action != SKIPLIST_DENY) {
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
  ui->printStr(1, 1, "Directory: " + release);
  ui->printStr(3, 1, "Section: ");
  bool highlight;
  for (unsigned int i = 0; i < msos.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = msos.getElement(i);
    highlight = false;
    if (msos.isFocused() && msos.getSelectionPointer() == i) {
      highlight = true;
    }
    if (msoe->getId() == 0) {
      ui->printStr(msoe->getRow(), msoe->getCol(), getSectionButtonText(msoe), highlight);
    }
    else {
      ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
      ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
    }
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
  redraw();
}

bool NewRaceScreen::keyPressed(unsigned int ch) {
  infotext = "";
  unsigned int pagerows = (unsigned int) (row - 6) * 0.6;
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
      if (focusedarea->goUp() || focusedarea->goPrevious()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &msos;
          focusedarea->enterFocusFrom(2);
        }
        ui->update();
      }
      return true;
    case KEY_DOWN:
      if (focusedarea->goDown() || focusedarea->goNext()) {
        if (!focusedarea->isFocused()) {
          defocusedarea = focusedarea;
          focusedarea = &mso;
          focusedarea->enterFocusFrom(0);
        }
        ui->update();
      }
      return true;
    case KEY_LEFT:
      if (focusedarea->goLeft()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
      }
      return true;
    case KEY_RIGHT:
      if (focusedarea->goRight()) {
        if (!focusedarea->isFocused()) {
          // shouldn't happen
        }
        ui->update();
      }
      return true;
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
      return true;
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
      return true;
    case 10:

      activation = focusedarea->getElement(focusedarea->getSelectionPointer())->activate();
      if (!activation) {
        if (focusedarea == &msos) {
          Pointer<MenuSelectOptionElement> msoe = msos.getElement(msos.getSelectionPointer());
          if (msoe->getId() == 0) {
            section = msoe->getIdentifier();
            tempsites.clear();
            mso.clear();
          }
        }
        ui->redraw();
        return true;
      }
      active = true;
      activeelement = focusedarea->getElement(focusedarea->getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      return true;
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
    case 's':
    {
      Pointer<Race> race = startRace();
      if (!!race) {
        ui->returnRaceStatus(race->getId());
      }
      return true;
    }
    case 'S':
    {
      Pointer<Race> race = startRace();
      if (!!race) {
        ui->returnToLast();
        ui->addTempLegendSpreadJob(race->getId());
      }
      return true;
    }
    case 't':
      toggleall = !toggleall;
      populateSiteList();
      ui->redraw();
      return true;
  }
  return false;
}

std::string NewRaceScreen::getLegendText() const {
  return currentlegendtext;
}

std::string NewRaceScreen::getInfoLabel() const {
  return "START NEW SPREAD JOB";
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

Pointer<Race> NewRaceScreen::startRace() {
  msota->getData();
  std::list<std::string> sites;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionCheckBox> msocb = mso.getElement(i);
    if (msocb->getData()) {
      sites.push_back(msocb->getIdentifier());
    }
  }
  if (sites.size() < 2) {
    infotext = "Cannot start spread job with less than 2 sites!";
    ui->update();
    return Pointer<Race>();
  }
  if (msota->getData() == SPREAD_RACE) {
    return global->getEngine()->newRace(release, section, sites);
  }
  else {
    return global->getEngine()->newDistribute(release, section, sites);
  }
}
