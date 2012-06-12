#include "menuselectoptiontextfield.h"

MenuSelectOptionTextField::MenuSelectOptionTextField(std::string identifier, int row, int col, std::string label, std::string starttext, int visiblelen, int maxlen, bool secret) {
  textfield = TextInputField(starttext, visiblelen, maxlen, secret);
  init(identifier, row, col, label);
}

std::string MenuSelectOptionTextField::getContentText() {
  return textfield.getVisualText();
}

void MenuSelectOptionTextField::inputChar(int ch) {
  if (ch >= 32 && ch <= 126) {
      textfield.addchar(ch);
  }
  else if (ch == KEY_BACKSPACE) {
    textfield.eraseLast();
  }
}

int MenuSelectOptionTextField::cursorPosition() {
  return textfield.getLastCharPosition();
}

std::string MenuSelectOptionTextField::getData() {
  return textfield.getText();
}
