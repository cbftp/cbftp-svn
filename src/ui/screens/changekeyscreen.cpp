#include "changekeyscreen.h"

#include "../../globalcontext.h"
#include "../../datafilehandler.h"

#include "../uicommunicator.h"
#include "../menuselectoptionelement.h"
#include "../menuselectoptiontextfield.h"
#include "../termint.h"

extern GlobalContext * global;

ChangeKeyScreen::ChangeKeyScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one - [c]ancel";
  currentlegendtext = defaultlegendtext;
  active = false;
  mismatch = false;
  oldmismatch = false;
  tooshort = false;
  unsigned int y = 4;
  unsigned int x = 1;

  mso.addStringField(y++, x, "oldkey", "Old passphrase:", "", true);
  mso.addStringField(y++, x, "newkey", "New passphrase:", "", true);
  mso.addStringField(y++, x, "newkey2", "Verify new:", "", true);
  init(window, row, col);
}

void ChangeKeyScreen::redraw() {
  werase(window);
  unsigned int y = 1;
  TermInt::printStr(window, y, 1, "Please verify with your old encryption key.");
  bool highlight;
  for (unsigned int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
    if (highlight) wattroff(window, A_REVERSE);
    TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  }
}

void ChangeKeyScreen::update() {
  if (mismatch || oldmismatch || tooshort) {
    redraw();
  }
  MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  msoe = mso.getElement(mso.getSelectionPointer());
  wattron(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText());
  wattroff(window, A_REVERSE);
  TermInt::printStr(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText());
  std::string error = "                                                          ";
  if (tooshort) {
    error = "Failed: The passphrase must be at least " + global->int2Str(SHORTESTKEY) + " characters long.";
  }
  else if (mismatch) {
    error = "Failed: The new keys did not match.";
  }
  else if (oldmismatch) {
    error = "Failed: the old key was not correct.";
  }
  TermInt::printStr(window, 8, 1, error);

  if (active && msoe->cursorPosition() >= 0) {
    curs_set(1);
    TermInt::moveCursor(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void ChangeKeyScreen::keyPressed(unsigned int ch) {
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
  switch(ch) {
    case KEY_UP:
      mso.goUp();
      uicommunicator->newCommand("update");
      break;
    case KEY_DOWN:
      mso.goDown();
      uicommunicator->newCommand("update");
      break;
    case 10:

      activation = mso.getElement(mso.getSelectionPointer())->activate();
      tooshort = false;
      mismatch = false;
      oldmismatch = false;
      if (!activation) {
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 'c':
      uicommunicator->newCommand("return");
      break;
    case 'd':
      MenuSelectOptionTextField * field1 = (MenuSelectOptionTextField *)mso.getElement(0);
      MenuSelectOptionTextField * field2 = (MenuSelectOptionTextField *)mso.getElement(1);
      MenuSelectOptionTextField * field3 = (MenuSelectOptionTextField *)mso.getElement(2);
      std::string oldkey = field1->getData();
      std::string newkey = field2->getData();
      std::string newkey2 = field3->getData();
      field1->clear();
      field2->clear();
      field3->clear();
      if (newkey == newkey2) {
        if (newkey.length() >= SHORTESTKEY) {
          if (global->getDataFileHandler()->changeKey(oldkey, newkey)) {
            uicommunicator->newCommand("return");
            break;
          }
          else {
            oldmismatch = true;
          }
        }
        else {
          tooshort = true;
        }
      }
      else {
        mismatch = true;
      }
      uicommunicator->newCommand("update");
      break;
  }
}

std::string ChangeKeyScreen::getLegendText() {
  return currentlegendtext;
}

std::string ChangeKeyScreen::getInfoLabel() {
  return "CHANGE ENCRYPTION KEY";
}
