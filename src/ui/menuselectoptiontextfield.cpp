#include "menuselectoptiontextfield.h"

MenuSelectOptionTextField::MenuSelectOptionTextField() {
  textfield = TextInputField("", 0, 0, false);
  init("none", 0, 0, "none");
}
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
  else if (ch == KEY_BACKSPACE || ch == 8) {
    textfield.eraseLast();
  }
}

int MenuSelectOptionTextField::cursorPosition() {
  return textfield.getLastCharPosition();
}

std::string MenuSelectOptionTextField::getData() {
  return textfield.getText();
}

void MenuSelectOptionTextField::clear() {
  textfield.clear();
}

std::string MenuSelectOptionTextField::getLegendText() {
  return "[Enter] Finish editing - [Any] Input to text";
}
