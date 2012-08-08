#include "loginscreen.h"

LoginScreen::LoginScreen(WINDOW * window, UICommunicator * uicommunicator, int row, int col) {
  this->uicommunicator = uicommunicator;
  passfield = TextInputField(25, 32, true);
  attempt = false;
  init(window, row, col);
}

void LoginScreen::redraw() {
  werase(window);
  pass_row = row-2;
  pass_col = col-27;
  curs_set(1);
  std::string svnstring = " This is Project Clusterbomb. Version tag: " + std::string(VERSION) + " ";
  std::string compilestring = " Compiled: " + std::string(BUILDTIME) + " ";
  int boxchar = 0;
  for(int i = 1; i < row; i++) {
    for(int j = 0; j < col; j++) {
      if(i == 1) boxchar = (i+j)%2==0 ? 4194423 : 4194417;
      else if (i == row-1) {
        if (j < col-29) boxchar = (i+j)%2==0 ? 4194417 : 4194422;
        else if (j == col-29) boxchar = 4194410;
        else continue;
      }
      else if ((i == row-2 || i == row-3) && j >= col-29) {
        if (j == col-29) boxchar = (i+j)%2==0 ? 4194424 : 4194421;
        else continue;
      }
      else if (i == row-4 && j >= col-29) {
        if (j == col-29) boxchar = (i+j)%2==0 ? 4194412 : 4194414;
        else boxchar = (i+j)%2==0 ? 4194417 : 4194422;
      }
      else boxchar = (i+j)%2==0 ? 4194412 : 4194410;
      if (boxchar) TermInt::printChar(window, i, j, boxchar);
    }
  }
  TermInt::printStr(window, 0, 3, svnstring);
  TermInt::printStr(window, 0, col - compilestring.length() - 3, compilestring);
  update();

}

void LoginScreen::update() {
  std::string passtext = "AES passphrase required:";
  if (attempt) {
    passtext = "Invalid key, try again: ";
  }
  TermInt::printStr(window, pass_row-1, pass_col, passtext);
  TermInt::printStr(window, pass_row, pass_col, passfield.getVisualText());
  TermInt::moveCursor(window, pass_row, pass_col + passfield.getLastCharPosition());
}

void LoginScreen::keyPressed(int ch) {
  if (ch >= 32 && ch <= 126) {
    passfield.addchar(ch);
  }
  else {
    switch(ch) {
      case KEY_BACKSPACE:
        passfield.eraseLast();
        break;
      case KEY_ENTER:
      case 10:
      case 13:
        curs_set(0);
        attempt = true;
        uicommunicator->newCommand("key", passfield.getText());
        passfield.clear();
        return;
    }
  }
  uicommunicator->newCommand("update");
}




