#include "textinputfield.h"

TextInputField::TextInputField() {

}

TextInputField::TextInputField(int visiblelen, int maxlen) {
  construct("", visiblelen, maxlen, false);
}

TextInputField::TextInputField(int visiblelen, int maxlen, bool secret) {
  construct("", visiblelen, maxlen, secret);
}

TextInputField::TextInputField(std::string starttext, int visiblelen, int maxlen, bool secret) {
  construct(starttext, visiblelen, maxlen, secret);
}

void TextInputField::construct(std::string starttext, int visiblelen, int maxlen, bool secret) {
  this->text = starttext;
  this->visiblelen = visiblelen;
  this->maxlen = maxlen;
  this->secret = secret;
}

std::string TextInputField::getText() {
  return text;
}

std::string TextInputField::getVisualText() {
  std::string visualtext = "";
  int writelen = text.length();
  int start = 0;
  if (writelen > visiblelen) {
    start = writelen - visiblelen;
    writelen = visiblelen;
  }
  for (int i = 0; i < writelen; i++) {
    visualtext += secret ? '*' : text[start+i];
  }
  for (int i = writelen; i < visiblelen; i++) {
    visualtext += ' ';
  }
  return visualtext;
}

int TextInputField::getLastCharPosition() {
  int end = text.length();
  if (end > visiblelen) {
    return visiblelen;
  }
  return end;
}

bool TextInputField::addchar(char c) {
  if (text.length() < maxlen) {
    text += c;
    return true;
  }
  return false;
}

void TextInputField::eraseLast() {
  int len = text.length();
  if (len > 0) {
    text = text.substr(0, len-1);
  }
}

void TextInputField::clear() {
  text = "";
}
