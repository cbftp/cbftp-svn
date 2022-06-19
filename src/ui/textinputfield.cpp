#include "textinputfield.h"

TextInputField::TextInputField() {

}

TextInputField::TextInputField(int visiblelen, int maxlen) {
  construct("", visiblelen, maxlen, false);
}

TextInputField::TextInputField(int visiblelen, int maxlen, bool secret) {
  construct("", visiblelen, maxlen, secret);
}

TextInputField::TextInputField(const FmtString& starttext, int visiblelen, int maxlen, bool secret) {
  construct(starttext, visiblelen, maxlen, secret);
}

void TextInputField::construct(const FmtString& starttext, int visiblelen, int maxlen, bool secret) {
  this->text = starttext;
  this->visiblelen = visiblelen;
  this->maxlen = maxlen;
  this->secret = secret;
  this->cursor = starttext.length();
}

FmtString TextInputField::getText() const {
  return text;
}

FmtString TextInputField::getVisualText() const {
  FmtString visualtext = "";
  unsigned int writelen = text.length();
  unsigned int start = 0;
  if (writelen > visiblelen) {
    if (cursor >= writelen - visiblelen / 2) {
      start = writelen - visiblelen;
    }
    else if (cursor >= visiblelen / 2) {
      start = cursor - visiblelen / 2;
    }
    else {
      start = 0;
    }
    writelen = visiblelen;
  }
  if (secret) {
    visualtext = std::string(writelen, '*');
  }
  else {
    visualtext = text.substr(start, writelen);
  }
  if (visiblelen > writelen) {
    visualtext += std::string(visiblelen - writelen, ' ');
  }
  return visualtext;
}

unsigned int TextInputField::getVisualCursorPosition() const {
  unsigned int writelen = text.length();
  if (writelen > visiblelen) {
    if (cursor >= writelen - visiblelen / 2) {
      return cursor + visiblelen - writelen;
    }
    else if (cursor >= visiblelen / 2) {
      return visiblelen / 2;
    }
    else {
      return cursor;
    }
  }
  return cursor;
}

bool TextInputField::addchar(char c) {
  size_t len = text.length();
  if (len >= maxlen) {
    return false;
  }
  unsigned int rawpos = text.positionInRaw(cursor);
  text = text.rawSubstr(0, rawpos) + c + text.rawSubstr(rawpos);
  cursor = text.positionInFormatted(rawpos + 1);
  return true;
}

bool TextInputField::moveCursorLeft() {
  if (cursor > 0) {
    cursor--;
    return true;
  }
  return false;
}

bool TextInputField::moveCursorRight() {
  if (cursor < text.length()) {
    cursor++;
    return true;
  }
  return false;
}

void TextInputField::moveCursorHome() {
  cursor = 0;
}

void TextInputField::moveCursorEnd() {
  cursor = text.length();
}

void TextInputField::erase() {
  if (text.rawLength() == 0) {
    return;
  }
  unsigned int pos = text.positionInRaw(cursor);
  if (pos == 0) {
    return;
  }
  int len = text.length();
  text = text.rawSubstr(0, pos - 1) + text.rawSubstr(pos);
  cursor = cursor - (len - text.length());
}

void TextInputField::eraseForward() {
  if (text.rawLength() == 0) {
    return;
  }
  unsigned int pos = text.positionInRaw(cursor, false);
  if (pos == text.rawLength()) {
    return;
  }
  text = text.rawSubstr(0, pos) + text.rawSubstr(pos + 1);
}

void TextInputField::eraseCursoredWord() {
  if (text.empty() || !cursor) {
    return;
  }
  unsigned int erasestart = cursor - 1;
  bool foundword = false;
  for (;; --erasestart) {
    bool erase = false;
    if (text[erasestart] == ' ' && foundword) {
      ++erasestart;
      erase = true;
    }
    if (erase || erasestart == 0) {
      FmtString newtext = text.substr(0, erasestart) + text.substr(cursor);
      cursor -= (text.length() - newtext.length());
      text = newtext;
      return;
    }
    if (text[erasestart] != ' ') {
      foundword = true;
    }
  }
}

void TextInputField::eraseAllBefore() {
  while (cursor > 0) {
    erase();
  }
}

void TextInputField::moveCursorPreviousWord() {
  bool alphanumpassed = false;
  while (cursor > 0) {
    if (isalnum(text[cursor - 1])) {
      alphanumpassed = true;
    }
    else if (alphanumpassed) {
      return;
    }
    cursor--;
  }
}

void TextInputField::moveCursorNextWord() {
  bool alphanumpassed = false;
  while (cursor < text.length()) {
    if (isalnum(text[cursor])) {
      alphanumpassed = true;
    }
    else if (alphanumpassed) {
      return;
    }
    cursor++;
  }
}

void TextInputField::setText(const FmtString& text) {
  this->text = text;
  moveCursorEnd();
}

void TextInputField::clear() {
  text = "";
  cursor = 0;
}

void TextInputField::setVisibleLength(unsigned int visiblelen) {
  this->visiblelen = visiblelen;
}
