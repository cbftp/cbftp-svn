#pragma once

#include <string>
#include <list>

#include "delayedcommand.h"

class ConnStateTracker {
private:
  int state;
  int time;
  std::list<std::string> releasedcommands;
  std::list<DelayedCommand> delayedcommands;
public:
  ConnStateTracker();
  void delayedCommand(std::string, int);
  void timePassed(int);
  int getTimePassed();
  bool hasReleasedCommand();
  std::string getCommand();
  void setDisconnected();
  void setIdle();
  void setReady();
  void setBusy();
  bool isDisconnected();
  bool isIdle();
  bool isReady();
};
