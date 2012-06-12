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
  int getLastCharPosition();
  bool addchar(char);
  void eraseLast();
  void clear();
private:
  void construct(std::string, int, int, bool);
  std::string text;
  bool secret;
  int maxlen;
  int visiblelen;
};
