#pragma once

#include <list>
#include <unistd.h>

#include "globalcontext.h"
#include "eventreceiver.h"
#include "tickpoketarget.h"
#include "workmanager.h"

extern GlobalContext * global;

#define SLEEPINTERVAL 50

class TickPoke : private EventReceiver {
private:
  WorkManager * wm;
  std::list<TickPokeTarget> targets;
  bool forever;
public:
  TickPoke();
  void tick(int);
  void tickerLoop();
  void breakLoop();
  void startPoke(EventReceiver *, int, int);
  void stopPoke(EventReceiver *, int);
};
