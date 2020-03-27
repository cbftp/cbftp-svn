#include "newkeyscreen.h"

#include "../ui.h"
#include "../menuselectoptiontextfield.h"
#include "../menuselectoptionelement.h"

#define MINIMUM_KEY_LENGTH 4

NewKeyScreen::NewKeyScreen(Ui* ui) : UIWindow(ui, "NewKeyScreen") {
  keybinds.addBind(10, KEYACTION_ENTER, "Modify");
  keybinds.addBind(KEY_DOWN, KEYACTION_DOWN, "Next option");
  keybinds.addBind(KEY_UP, KEYACTION_UP, "Previous option");
  keybinds.addBind('d', KEYACTION_DONE, "Done");
  keybinds.addBind('c', KEYACTION_BACK_CANCEL, "Cancel");
}

NewKeyScreen::~NewKeyScreen() {

}

void NewKeyScreen::initialize(unsigned int row, unsigned int col) {
  active = false;
  mismatch = false;
  tooshort = false;
  unsigned int y = 9;
  unsigned int x = 1;
  mso.clear();
  mso.addStringField(y++, x, "newkey", "Passphrase:", "", true);
  mso.addStringField(y++, x, "newkey2", "Verify:", "", true);
  init(row, col);
}

void NewKeyScreen::redraw() {
  ui->erase();
  unsigned int y = 1;
  ui->printStr(y, 1, "Your site and configuration data will be encrypted with AES-256-CBC.");
  ui->printStr(y+1, 1, "A 256-bit (32 characters) AES key will be generated from the given passphrase.");
  ui->printStr(y+2, 1, "This means that the level of security increases with the length of the given");
  ui->printStr(y+3, 1, "passphrase.");
  ui->printStr(y+4, 1, "The passphrase is not considered secure if it is shorter than 16 characters.");
  ui->printStr(y+6, 1, "Good password practice is described well by xkcd: http://xkcd.com/936/");
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

void NewKeyScreen::update() {
  std::shared_ptr<MenuSelectOptionElement> msoe = mso.getElement(mso.getLastSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  ui->printStr(msoe->getRow(), msoe->getCol(), msoe->getLabelText(), true);
  ui->printStr(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  std::string error = "                                                          ";
  if (tooshort) {
    error = "Failed: The passphrase must be at least " + std::to_string(MINIMUM_KEY_LENGTH) + " characters long.";
  }
  else if (mismatch) {
    error = "Failed: The keys did not match.";
  }
  ui->printStr(14, 1, error);

  if (active && msoe->cursorPosition() >= 0) {
    ui->showCursor();
    ui->moveCursor(msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    ui->hideCursor();
  }
}

bool NewKeyScreen::keyPressed(unsigned int ch) {
  int action = keybinds.getKeyAction(ch);
  if (active) {
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      ui->update();
      ui->setLegend();
      return true;
    }
    activeelement->inputChar(ch);
    ui->update();
    return true;
  }
  bool activation;
  switch(action) {
    case KEYACTION_UP:
      mso.goUp();
      ui->update();
      return true;
    case KEYACTION_DOWN:
      mso.goDown();
      ui->update();
      return true;
    case KEYACTION_ENTER:
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      tooshort = false;
      mismatch = false;
      if (!activation) {
        ui->update();
        return true;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      ui->update();
      ui->setLegend();
      return true;
    case KEYACTION_DONE: {
      std::shared_ptr<MenuSelectOptionTextField> field1 = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement(0));
      std::shared_ptr<MenuSelectOptionTextField> field2 = std::static_pointer_cast<MenuSelectOptionTextField>(mso.getElement(1));
      std::string key = field1->getData();
      std::string key2 = field2->getData();
      field1->clear();
      field2->clear();
      if (key == key2) {
        if (key.length() >= MINIMUM_KEY_LENGTH) {
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
    case KEYACTION_BACK_CANCEL:
      ui->returnToLast();
      return true;
  }
  return false;
}

std::string NewKeyScreen::getLegendText() const {
  if (active) {
    return activeelement->getLegendText();
  }
  return keybinds.getLegendSummary();
}

std::string NewKeyScreen::getInfoLabel() const {
  return "ENABLE ENCRYPTION";
}
