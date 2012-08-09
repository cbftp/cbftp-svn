#pragma once

#include <semaphore.h>

class TickPokeTarget {
private:
  sem_t * pokee;
  int interval;
  int currentval;
  int message;
public:
  TickPokeTarget(sem_t *, int, int);
  int getMessage();
  sem_t * getPokee();
  bool tick(int);
};
