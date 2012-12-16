#pragma once

#include <string>

class DelayedCommand {
private:
  std::string command;
  int delay;
  void * arg;
public:
  DelayedCommand(std::string, int);
  DelayedCommand(std::string, int, void *);
  DelayedCommand(const DelayedCommand &);
  std::string getCommand();
  int getDelay();
  void * getArg();
};
