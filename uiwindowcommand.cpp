#include "uiwindowcommand.h"

UIWindowCommand::UIWindowCommand() {
  newcommand = false;
}

void UIWindowCommand::newCommand(std::string command) {
  newCommand(command, "", "");
}

void UIWindowCommand::newCommand(std::string command, std::string arg1) {
  newCommand(command, arg1, "");
}

void UIWindowCommand::newCommand(std::string command, std::string arg1, std::string arg2) {
  this->command = command;
  this->arg1 = arg1;
  this->arg2 = arg2;
  newcommand = true;
}

void UIWindowCommand::checkoutCommand() {
  newcommand = false;
}

bool UIWindowCommand::hasNewCommand() {
  return newcommand;
}

std::string UIWindowCommand::getCommand() {
  return command;
}

std::string UIWindowCommand::getArg1() {
  return arg1;
}

std::string UIWindowCommand::getArg2() {
  return arg2;
}
