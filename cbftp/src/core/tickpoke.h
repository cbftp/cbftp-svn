#pragma once

#include <list>

#include "eventreceiver.h"

#define SLEEPINTERVAL 50

class TickPokeTarget;
class WorkManager;

class TickPoke : private EventReceiver {
private:
  WorkManager * wm;
  std::list<TickPokeTarget> targets;
  bool forever;
public:
  TickPoke(WorkManager *);
  void tick(int);
  void tickerLoop();
  void breakLoop();
  void startPoke(EventReceiver *, std::string, int, int);
  void stopPoke(EventReceiver *, int);
};
