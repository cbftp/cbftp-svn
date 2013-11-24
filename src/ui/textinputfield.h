#pragma once

#include <string>

class TextInputField {
public:
  TextInputField();
  TextInputField(int, int);
  TextInputField(int, int, bool);
  TextInputField(std::string, int, int, bool);
  std::string getText();
  std::string getVisualText();
  unsigned int getVisualCursorPosition();
  bool addchar(char);
  void erase();
  bool moveCursorLeft();
  bool moveCursorRight();
  void moveCursorHome();
  void moveCursorEnd();
  void setText(std::string);
  void clear();
private:
  void construct(std::string, int, int, bool);
  std::string text;
  unsigned int cursor;
  bool secret;
  unsigned int maxlen;
  unsigned int visiblelen;
};
