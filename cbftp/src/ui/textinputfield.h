#pragma once

#include "fmtstring.h"

class TextInputField {
public:
  TextInputField();
  TextInputField(int, int);
  TextInputField(int, int, bool);
  TextInputField(const FmtString& starttext, int, int, bool);
  FmtString getText() const;
  FmtString getVisualText() const;
  unsigned int getVisualCursorPosition() const;
  bool addchar(char);
  void erase();
  void eraseForward();
  void eraseCursoredWord();
  void eraseAllBefore();
  bool moveCursorLeft();
  bool moveCursorRight();
  void moveCursorHome();
  void moveCursorEnd();
  void moveCursorPreviousWord();
  void moveCursorNextWord();
  void setText(const FmtString& text);
  void clear();
  void setVisibleLength(unsigned int);
private:
  void construct(const FmtString& starttext, int, int, bool);
  FmtString text;
  unsigned int cursor;
  bool secret;
  unsigned int maxlen;
  unsigned int visiblelen;
};
