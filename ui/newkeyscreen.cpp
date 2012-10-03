#include "newkeyscreen.h"

NewKeyScreen::NewKeyScreen(WINDOW * window, UICommunicator * uicommunicator, unsigned int row, unsigned int col) {
  this->uicommunicator = uicommunicator;
  defaultlegendtext = "[Enter] Modify - [Down] Next option - [Up] Previous option - [d]one";
  currentlegendtext = defaultlegendtext;
  active = false;
  mismatch = false;
  tooshort = false;
  unsigned int y = 11;
  unsigned int x = 1;

  mso.addStringField(y++, x, "newkey", "Passphrase:", "", true);
  mso.addStringField(y++, x, "newkey2", "Verify:", "", true);
  init(window, row, col);
}

void NewKeyScreen::redraw() {
  werase(window);
  TermInt::printStr(window, 1, 1, "-== CREATE DATA FILE ==-");
  TermInt::printStr(window, 3, 1, "Welcome!");
  TermInt::printStr(window, 5, 1, "Your site and configuration data will be encrypted with AES-256-CBC.");
  TermInt::printStr(window, 6, 1, "A 256-bit (32 characters) AES key will be generated from the given passphrase.");
  TermInt::printStr(window, 7, 1, "This means that the level of security increases with the length of the given");
  TermInt::printStr(window, 8, 1, "passphrase.");
  TermInt::printStr(window, 9, 1, "The passphrase is not considered secure if it is shorter than 16 characters.");
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

void NewKeyScreen::update() {
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
    error = "Failed: The keys did not match.";
  }
  TermInt::printStr(window, 14, 1, error);

  if (active && msoe->cursorPosition() >= 0) {
    curs_set(1);
    TermInt::moveCursor(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void NewKeyScreen::keyPressed(unsigned int ch) {
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
      mso.goPrev();
      uicommunicator->newCommand("update");
      break;
    case KEY_DOWN:
      mso.goNext();
      uicommunicator->newCommand("update");
      break;
    case 10:

      activation = mso.getElement(mso.getSelectionPointer())->activate();
      tooshort = false;
      mismatch = false;
      if (!activation) {
        uicommunicator->newCommand("update");
        break;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
      currentlegendtext = activeelement->getLegendText();
      uicommunicator->newCommand("updatesetlegend");
      break;
    case 'd':
      MenuSelectOptionTextField * field1 = (MenuSelectOptionTextField *)mso.getElement(0);
      MenuSelectOptionTextField * field2 = (MenuSelectOptionTextField *)mso.getElement(1);
      std::string key = field1->getData();
      std::string key2 = field2->getData();
      field1->clear();
      field2->clear();
      if (key == key2) {
        if (key.length() >= SHORTESTKEY) {
          uicommunicator->newCommand("newkey", key);
          break;
        }
        tooshort = true;
      }
      else {
        mismatch = true;
      }
      uicommunicator->newCommand("update");
      break;
  }
}

std::string NewKeyScreen::getLegendText() {
  return currentlegendtext;
}
