#pragma once

#include <map>
#include <string>

class UIWindowCommand {
private:
  bool newcommand;
  std::string command;
  std::string arg1;
  std::string arg2;
public:
  UIWindowCommand();
  void newCommand(std::string);
  void newCommand(std::string, std::string);
  void newCommand(std::string, std::string, std::string);
  void checkoutCommand();
  bool hasNewCommand();
  std::string getCommand();
  std::string getArg1();
  std::string getArg2();
};
