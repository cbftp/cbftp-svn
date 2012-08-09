#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <list>
#include <map>

#include "globalcontext.h"
#include "tickpoketarget.h"

extern GlobalContext * global;

#define SLEEPINTERVAL 50

class TickPoke {
private:
  pthread_t thread;
  static void * run(void *);
  std::list<TickPokeTarget> targets;
  std::map<sem_t *, std::list<int> > pokes;
public:
  TickPoke();
  void runInstance();
  void startPoke(sem_t *, int, int);
  void stopPoke(sem_t *, int);
  int getMessage(sem_t *);
  bool isPoked(sem_t *);
};
