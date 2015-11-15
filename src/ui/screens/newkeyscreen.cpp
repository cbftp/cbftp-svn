#include "newkeyscreen.h"

#include "../../util.h"

#include "../ui.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"

NewKeyScreen::NewKeyScreen(Ui * ui) {
  this->ui = ui;
}

NewKeyScreen::~NewKeyScreen() {

}

void NewKeyScreen::initialize(unsigned int row, unsigned int col) {
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one";
  currentlegendtext = defaultlegendtext;
  active = false;
  mismatch = false;
  tooshort = false;
  unsigned int y = 11;
  unsigned int x = 1;
  mso.clear();
  mso.addStringField(y++, x, "newkey", "Passphrase:", "", true);
  mso.addStringField(y++, x, "newkey2", "Verify:", "", true);
  init(row, col);
}

void NewKeyScreen::redraw() {
  ui->erase();
  unsigned int y = 1;
  ui->printStr(y, 1, "Welcome!");
  ui->printStr(y+2, 1, "Your site and configuration data will be encrypted with AES-256-CBC.");
  ui->printStr(y+3, 1, "A 256-bit (32 characters) AES key will be generated from the given passphrase.");
  ui->printStr(y+4, 1, "This means that the level of security increases with the length of the given");
  ui->printStr(y+5, 1, "passphrase.");
  ui->printStr(y+6, 1, "The passphrase is not considered secure if it is shorter than 16 characters.");
  ui->printStr(y+8, 1, "Good password practice is described well by xkcd: http://xkcd.com/936/");
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    Pointer<MenuSelectOptionElement> msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), highlight);
    ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void NewKeyScreen::update() {
  Pointer<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  std::string error = "                                                          ";
  if (tooshort) {
    error = "Failed: The passphrase must be at least " + util::int2Str(SHORTESTKEY) + " characters long.";
  }
  else if (mismatch) {
    error = "Failed: The keys did not match.";
  }
  ui->printStr(16, 1, error);

  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    ui->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool NewKeyScreen::keyPressed(unsigned int ch) {
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
      mso.goUp();
      ui->update();
      return true;
    case KEY_DOWN:
      mso.goDown();
      ui->update();
      return true;
    case 10:

      activation = mso.getElement(mso.getSelectionPointer())->activate();
      tooshort = false;
      mismatch = false;
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      ui->update();
      ui->setLegend();
      return true;
    case 'd':
      Pointer<MenuSelectOptionTextField> field1 = mso.getElement(0);
      Pointer<MenuSelectOptionTextField> field2 = mso.getElement(1);
      std::string key = field1->getData();
      std::string key2 = field2->getData();
      field1->clear();
      field2->clear();
      if (key == key2) {
        if (key.length() >= SHORTESTKEY) {
          ui->newKey(key);
          return true;
        }
        tooshort = true;
      }
      else {
        mismatch = true;
      }
      ui->update();
      return true;
  }
  return false;
}

std::string NewKeyScreen::getLegendText() const {
  return currentlegendtext;
}

std::string NewKeyScreen::getInfoLabel() const {
  return "CREATE DATA FILE";
}
