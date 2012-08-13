#pragma once

#include <string>

class DelayedCommand {
private:
  std::string command;
  int delay;
public:
  DelayedCommand(std::string, int);
  std::string getCommand();
  int getDelay();
};
