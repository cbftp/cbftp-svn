#include "newkeyscreen.h"

NewKeyScreen::NewKeyScreen(WINDOW * window, UIWindowCommand * windowcommand, int row, int col) {
  this->windowcommand = windowcommand;
  active = false;
  mismatch = false;
  tooshort = false;
  int y = 11;
  int x = 1;

  mso.addStringField(y++, x, "newkey", "Passphrase:", "", true);
  mso.addStringField(y++, x, "newkey2", "Verify:", "", true);
  init(window, row, col);
}

void NewKeyScreen::redraw() {
  werase(window);
  mvwprintw(window, 1, 1, "-== CREATE DATA FILE ==-");
  mvwprintw(window, 3, 1, "Welcome!");
  mvwprintw(window, 5, 1, "Your site and configuration data will be encrypted with AES-256-CBC.");
  mvwprintw(window, 6, 1, "A 256-bit (32 characters) AES key will be generated from the given passphrase.");
  mvwprintw(window, 7, 1, "This means that the level of security increases with the length of the given");
  mvwprintw(window, 8, 1, "passphrase.");
  mvwprintw(window, 9, 1, "The passphrase is not considered secure if it is shorter than 16 characters.");
  bool highlight;
  for (int i = 0; i < mso.size(); i++) {
    MenuSelectOptionElement * msoe = mso.getElement(i);
    highlight = false;
    if (mso.getSelectionPointer() == i) {
      highlight = true;
    }
    if (highlight) wattron(window, A_REVERSE);
    mvwprintw(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText().c_str());
    if (highlight) wattroff(window, A_REVERSE);
    mvwprintw(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText().c_str());
  }
}

void NewKeyScreen::update() {
  MenuSelectOptionElement * msoe = mso.getElement(mso.getLastSelectionPointer());
  mvwprintw(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText().c_str());
  mvwprintw(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText().c_str());
  msoe = mso.getElement(mso.getSelectionPointer());
  wattron(window, A_REVERSE);
  mvwprintw(window, msoe->getRow(), msoe->getCol(), msoe->getLabelText().c_str());
  wattroff(window, A_REVERSE);
  mvwprintw(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1, msoe->getContentText().c_str());
  std::string error = "                                                          ";
  if (tooshort) {
    error = "Failed: The passphrase must be at least " + global->int2Str(SHORTESTKEY) + " characters long.";
  }
  else if (mismatch) {
    error = "Failed: The keys did not match.";
  }
  mvwprintw(window, 14, 1, error.c_str());

  if (active && msoe->cursorPosition() >= 0) {
    curs_set(1);
    wmove(window, msoe->getRow(), msoe->getCol() + msoe->getLabelText().length() + 1 + msoe->cursorPosition());
  }
  else {
    curs_set(0);
  }
}

void NewKeyScreen::keyPressed(int ch) {
  if (active) {
    windowcommand->newCommand("update");
    if (ch == 10) {
      activeelement->deactivate();
      active = false;
      return;
    }
    activeelement->inputChar(ch);
    return;
  }
  bool activation;
  switch(ch) {
    case KEY_UP:
      mso.goPrev();
      windowcommand->newCommand("update");
      break;
    case KEY_DOWN:
      mso.goNext();
      windowcommand->newCommand("update");
      break;
    case 10:
      windowcommand->newCommand("update");
      activation = mso.getElement(mso.getSelectionPointer())->activate();
      tooshort = false;
      mismatch = false;
      if (!activation) {
        break;
      }
      active = true;
      activeelement = mso.getElement(mso.getSelectionPointer());
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
          windowcommand->newCommand("newkey", key);
          break;
        }
        tooshort = true;
      }
      else {
        mismatch = true;
      }
      windowcommand->newCommand("update");
      break;
  }
}
