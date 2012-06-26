#include "loginscreen.h"

LoginScreen::LoginScreen(WINDOW * window, UIWindowCommand * windowcommand, int row, int col) {
  this->windowcommand = windowcommand;
  passfield = TextInputField(25, 32, true);
  pass_row = row-2;
  pass_col = col-27;
  attempt = false;
  init(window, row, col);
}

void LoginScreen::redraw() {
  werase(window);
  curs_set(1);
  std::string svnstring = " This is Project Clusterbomb SVN r" + std::string(SVNREV) + " ";
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
      if (boxchar) mvwaddch(window, i, j, boxchar);
    }
  }
  mvwprintw(window, 0, 3, svnstring.c_str());
  mvwprintw(window, 0, col - compilestring.length() - 3, compilestring.c_str());
  update();

}

void LoginScreen::update() {
  std::string passtext = "AES passphrase required:";
  if (attempt) {
    passtext = "Invalid key, try again: ";
  }
  mvwprintw(window, pass_row-1, pass_col, passtext.c_str());
  mvwprintw(window, pass_row, pass_col, passfield.getVisualText().c_str());
  wmove(window, pass_row, pass_col + passfield.getLastCharPosition());
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
        windowcommand->newCommand("key", passfield.getText());
        passfield.clear();
        return;
    }
  }
  windowcommand->newCommand("update");
}




