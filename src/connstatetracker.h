#pragma once

#include <string>
#include <list>

#include "delayedcommand.h"

class ConnStateTracker {
private:
  int state;
  int time;
  std::list<DelayedCommand> releasedcommands;
  std::list<DelayedCommand> delayedcommands;
public:
  ConnStateTracker();
  void delayedCommand(std::string, int);
  void delayedCommand(std::string, int, void *);
  void timePassed(int);
  int getTimePassed();
  bool hasReleasedCommand();
  DelayedCommand getCommand();
  void setDisconnected();
  void setIdle();
  void setReady();
  void setBusy();
  bool isDisconnected();
  bool isIdle();
  bool isReady();
};
