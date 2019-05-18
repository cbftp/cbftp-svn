#include "makedirscreen.h"

#include "../../globalcontext.h"
#include "../../filelist.h"

#include "../ui.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptiontextbutton.h"

MakeDirScreen::MakeDirScreen(Ui * ui) {
  this->ui = ui;
}

MakeDirScreen::~MakeDirScreen() {

}

void MakeDirScreen::initialize(unsigned int row, unsigned int col, const std::string & site, UIFileList & filelist) {
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [m]ake directory - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  this->site = site;
  this->filelist = filelist;
  alreadyexists = false;
  mso.reset();
  int y = 3;
  if (!site.empty()) {
    y++;
  }
  mso.addStringField(y, 1, "name", "Name:", "", false, col - 3, 512);
  mso.addTextButtonNoContent(y + 2, 1, "makedir", "Make directory");
  mso.addTextButtonNoContent(y + 2, 18, "cancel", "Cancel");
  mso.enterFocusFrom(0);
  init(row, col);
}

void MakeDirScreen::redraw() {
  ui->erase();
  int y = 1;
  if (!site.empty()) {
    ui->printStr(y++, 1, "Site: " + site);
  }
  ui->printStr(y, 1, "Path: " + filelist.getPath().toString());
  if (alreadyexists) {
    ui->printStr(y + 6, 1, "ERROR: an item with that name already exists!");
    alreadyexists = false;
  }
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
  if (active && activeelement->cursorPosition() >= 0) {
    ui->showCursor();
    ui->moveCursor(activeelement->getRow(), activeelement->getCol() + activeelement->getLabelText().length() + 1 + activeelement->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void MakeDirScreen::update() {
  redraw();
}

bool MakeDirScreen::keyPressed(unsigned int ch) {
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
        if (activeelement->getIdentifier() == "makedir") {
          tryMakeDir();
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
    case 'm': {
      tryMakeDir();
      return true;
    }
  }
  return false;
}

void MakeDirScreen::tryMakeDir() {
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement("name");
  std::string dirname = std::static_pointer_cast<MenuSelectOptionTextField>(msoe)->getData();
  if (dirname.empty()) {
    return;
  }
  if (filelist.contains(dirname)) {
    alreadyexists = true;
    ui->redraw();
    return;
  }
  ui->returnMakeDir(dirname);
  return;
}

std::string MakeDirScreen::getLegendText() const {
  return currentlegendtext;
}

std::string MakeDirScreen::getInfoLabel() const {
  return "MAKE NEW DIRECTORY";
}
