#include "tickpoketarget.h"

TickPokeTarget::TickPokeTarget(EventReceiver * pokee, int interval, int message, std::string desc) {
  this->pokee = pokee;
  this->interval = interval;
  this->message = message;
  this->currentval = 0;
  this->desc = desc;
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
