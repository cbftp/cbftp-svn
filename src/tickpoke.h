#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <list>
#include <map>
#include <unistd.h>

#include "globalcontext.h"
#include "eventreceiver.h"
#include "tickpoketarget.h"

extern GlobalContext * global;

#define SLEEPINTERVAL 50

class TickPoke {
private:
  pthread_t thread[2];
  sem_t tick;
  static void * runTicker(void *);
  static void * run(void *);
  std::list<TickPokeTarget> targets;
public:
  TickPoke();
  void runTickerInstance();
  void runInstance();
  void startPoke(EventReceiver *, int, int);
  void stopPoke(EventReceiver *, int);
};
