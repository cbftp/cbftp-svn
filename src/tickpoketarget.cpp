#include "tickpoketarget.h"

TickPokeTarget::TickPokeTarget(EventReceiver * pokee, int interval, int message) {
  this->pokee = pokee;
  this->interval = interval;
  this->message = message;
  this->currentval = 0;
}

int TickPokeTarget::getMessage() const {
  return message;
}

EventReceiver * TickPokeTarget::getPokee() const {
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
