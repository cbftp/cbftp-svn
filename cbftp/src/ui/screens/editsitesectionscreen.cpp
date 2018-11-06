#include "editsitesectionscreen.h"

#include "../../globalcontext.h"
#include "../../site.h"
#include "../../sitemanager.h"
#include "../../settingsloadersaver.h"
#include "../../path.h"

#include "../ui.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"

EditSiteSectionScreen::EditSiteSectionScreen(Ui * ui) {
  this->ui = ui;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
}

EditSiteSectionScreen::~EditSiteSectionScreen() {

}

void EditSiteSectionScreen::initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site, const Path & path) {
  mode = Mode::ADD;
  initialize(row, col, site, "", path);
}

void EditSiteSectionScreen::initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site, const std::string & section) {
  mode = Mode::EDIT;
  oldsection = section;
  Path path = site->getSectionPath(section);
  initialize(row, col, site, section, path);
}

void EditSiteSectionScreen::initialize(unsigned int row, unsigned int col, const std::shared_ptr<Site> & site, const std::string & section, const Path & path) {
  currentlegendtext = defaultlegendtext;
  active = false;
  modsite = site;
  unsigned int y = 1;
  unsigned int x = 1;
  mso.reset();
  mso.addStringField(y++, x, "name", "Name:", section, false);
  mso.addTextButtonNoContent(y++, x, "select", "Select name from list...");
  mso.addStringField(y++, x, "path", "Path:", path.toString(), false, 64);
  mso.enterFocusFrom(0);
  init(row, col);
}

void EditSiteSectionScreen::redraw() {
  ui->erase();
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void EditSiteSectionScreen::update() {
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
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
    ui->hideCursor();
  }
}

bool EditSiteSectionScreen::keyPressed(unsigned int ch) {
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      currentlegendtext = defaultlegendtext;
      ui->setLegend();
      ui->update();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      mso.goUp();
      ui->update();
      return true;
    case KEY_DOWN:
      mso.goDown();
      ui->update();
      return true;
    case 10: {
      std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getSelectionPointer());
      activation = msoe->activate();
      if (msoe->getIdentifier() == "select") {
        ui->goSelectSection();
        return true;
      }
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      ui->setLegend();
      ui->update();
      return true;
    }
    case 27: // esc
    case 'c':
      ui->returnToLast();
      return true;
    case 'd':
      std::shared_ptr<MenuSelectOptionTextField> nameelem = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("name"));
      std::shared_ptr<MenuSelectOptionTextField> pathelem = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement("path"));
      std::string name = nameelem->getData();
      std::string path = pathelem->getData();
      if (path.empty()) {
        return true;
      }
      if (mode == Mode::EDIT && name != oldsection) {
        modsite->removeSection(oldsection);
      }
      modsite->addSection(name, path);
      global->getSettingsLoaderSaver()->saveSettings();
      ui->returnToLast();
      return true;
  }
  return false;
}

void EditSiteSectionScreen::command(const std::string & command, const std::string & arg) {
  if (command == "returnselectitems") {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement("name");
    std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->setText(arg);
    mso.setPointer(msoe);
    ui->redraw();
  }
}

std::string EditSiteSectionScreen::getLegendText() const {
  return currentlegendtext;
}

std::string EditSiteSectionScreen::getInfoLabel() const {
  if (mode == Mode::EDIT) {
    return "EDIT SECTION ON " + modsite->getName() + ": " + oldsection;
  }
  return "ADD SECTION ON " + modsite->getName();
}
