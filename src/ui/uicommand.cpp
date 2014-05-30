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

int UICommand::getCommand() {
  return command;
}

WINDOW * UICommand::getWindow() {
  return window;
}

unsigned int UICommand::getRow() {
  return row;
}

unsigned int UICommand::getCol() {
  return col;
}

std::string UICommand::getText() {
  return text;
}

int UICommand::getMaxlen() {
  return maxlen;
}

bool UICommand::getRightAlign() {
  return rightalign;
}

unsigned int UICommand::getChar() {
  return c;
}

bool UICommand::getShow() {
  return show;
}
