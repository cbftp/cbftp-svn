#include "tickpoketarget.h"

TickPokeTarget::TickPokeTarget(sem_t * pokee, int interval, int message) {
  this->pokee = pokee;
  this->interval = interval;
  this->message = message;
  this->currentval = 0;
}

int TickPokeTarget::getMessage() {
  return message;
}

sem_t * TickPokeTarget::getPokee() {
  return pokee;
}

bool TickPokeTarget::tick(int msecs) {
  currentval += msecs;
  if (currentval >= interval) {
    currentval = currentval - interval;
    return true;
  }
  return false;
}
