#include "uicommand.h"

UICommand::UICommand(int command) {
  this->command = command;
}

UICommand::UICommand(int command, bool show) {
  this->command = command;
  this->show = show;
}

UICommand::UICommand(int command, WINDOW * window, unsigned int row, unsigned int col) {
  this->command = command;
  this->window = window;
  this->row = row;
  this->col = col;
}

UICommand::UICommand(int command, WINDOW * window) {
  this->command = command;
  this->window = window;
}

UICommand::UICommand(int command, WINDOW * window, unsigned int row, unsigned int col, std::string text, int maxlen, bool rightalign) {
  this->command = command;
  this->window = window;
  this->row = row;
  this->col = col;
  this->text = text;
  this->maxlen = maxlen;
  this->rightalign = rightalign;
}

UICommand::UICommand(int command, WINDOW * window, unsigned int row, unsigned int col, unsigned int c) {
  this->command = command;
  this->window = window;
  this->row = row;
  this->col = col;
  this->c = c;
}

int UICommand::getCommand() const {
  return command;
}

WINDOW * UICommand::getWindow() const {
  return window;
}

unsigned int UICommand::getRow() const {
  return row;
}

unsigned int UICommand::getCol() const {
  return col;
}

std::string UICommand::getText() const {
  return text;
}

int UICommand::getMaxlen() const {
  return maxlen;
}

bool UICommand::getRightAlign() const {
  return rightalign;
}

unsigned int UICommand::getChar() const {
  return c;
}

bool UICommand::getShow() const {
  return show;
}
