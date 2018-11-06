#include "../../globalcontext.h"
#include "../../section.h"
#include "../../sectionmanager.h"
#include "../../util.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptionnumarrow.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptiontextbutton.h"
#include "editsectionscreen.h"

EditSectionScreen::EditSectionScreen(Ui * ui) : section(nullptr) {
  this->ui = ui;
}

EditSectionScreen::~EditSectionScreen() {

}

void EditSectionScreen::initialize(unsigned int row, unsigned int col, const std::string & section) {
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  if (this->section != nullptr) {
    delete this->section;
  }
  if (section == "") {
    mode = Mode::ADD;
    this->section = new Section();
    oldname = "";
  }
  else {
    mode = Mode::EDIT;
    Section * editsection = global->getSectionManager()->getSection(section);
    util::assert(editsection != NULL);
    this->section = new Section(*editsection);
    oldname = editsection->getName();
  }
  mso.reset();
  mso.addStringField(1, 1, "name", "Section name:", this->section->getName(), false);
  mso.addTextButtonNoContent(3, 1, "skiplist", "Configure skiplist...");
  mso.enterFocusFrom(0);
  init(row, col);
}

void EditSectionScreen::redraw() {
  ui->erase();
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.isFocused() && mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void EditSectionScreen::update() {
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
    curs_set(0);
  }
}

bool EditSectionScreen::keyPressed(unsigned int ch) {
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
        if (activeelement->getIdentifier() == "skiplist") {
          ui->goSkiplist(&section->getSkipList());
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
    case 'd':
      done();
      return true;
  }
  return false;
}

void EditSectionScreen::done() {
  for(unsigned int i = 0; i < mso.size(); i++) {
    std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(i);
    std::string identifier = msoe->getIdentifier();
    if (identifier == "name") {
      std::string newname = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
      section->setName(newname);
    }
  }
  switch (mode) {
    case Mode::ADD:
      if (global->getSectionManager()->addSection(*section)) {
        ui->returnToLast();
      }
      break;
    case Mode::EDIT:
      if (global->getSectionManager()->replaceSection(*section, oldname)) {
        ui->returnToLast();
      }
      break;
  }
}

std::string EditSectionScreen::getLegendText() const {
  return currentlegendtext;
}

std::string EditSectionScreen::getInfoLabel() const {
  if (mode == Mode::ADD) {
    return "ADD SECTION";
  }
  return "EDIT SECTION: " + oldname;
}
