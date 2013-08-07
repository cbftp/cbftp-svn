#include "tickpoke.h"

TickPoke::TickPoke() {
  wm = global->getWorkManager();
  forever = true;
}

void TickPoke::tickerLoop() {
  while(forever) {
    usleep(SLEEPINTERVAL * 1000);
    wm->dispatchTick(this, SLEEPINTERVAL);
  }
}

void TickPoke::breakLoop() {
  forever = false;
}

void TickPoke::tick(int interval) {
  std::list<TickPokeTarget>::iterator it;
  for(it = targets.begin(); it != targets.end(); it++) {
    if (it->tick(interval)) {
      EventReceiver * er = it->getPokee();
      er->lock();
      er->tick(it->getMessage());
      er->unlock();
    }
  }
}

void TickPoke::startPoke(EventReceiver * pokee, int interval, int message) {
  targets.push_back(TickPokeTarget(pokee, interval, message));
}

void TickPoke::stopPoke(EventReceiver * pokee, int message) {
  std::list<TickPokeTarget>::iterator it;
  for(it = targets.begin(); it != targets.end(); it++) {
    if (it->getPokee() == pokee && it->getMessage() == message) {
      targets.erase(it);
      return;
    }
  }
}
