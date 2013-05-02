#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <list>
#include <map>
#include <unistd.h>

#include "globalcontext.h"
#include "eventreceiver.h"
#include "tickpoketarget.h"
#include "workmanager.h"

extern GlobalContext * global;

#define SLEEPINTERVAL 50

class TickPoke : private EventReceiver {
private:
  pthread_t thread;
  static void * run(void *);
  WorkManager * wm;
  std::list<TickPokeTarget> targets;
public:
  TickPoke();
  void tick(int);
  void runInstance();
  void startPoke(EventReceiver *, int, int);
  void stopPoke(EventReceiver *, int);
};
