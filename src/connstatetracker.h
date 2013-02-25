#pragma once

#include <string>
#include <list>

#include "delayedcommand.h"

class SiteRace;

class ConnStateTracker {
private:
  int state;
  int time;
  int lastcheckedcount;
  SiteRace * lastchecked;
  std::list<DelayedCommand> releasedcommands;
  std::list<DelayedCommand> delayedcommands;
public:
  ConnStateTracker();
  void delayedCommand(std::string, int);
  void delayedCommand(std::string, int, void *);
  void timePassed(int);
  int getTimePassed();
  void check(SiteRace *);
  SiteRace * lastChecked();
  int checkCount();
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
