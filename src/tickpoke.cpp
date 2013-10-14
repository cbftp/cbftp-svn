#include "tickpoke.h"

#include <unistd.h>

#include "globalcontext.h"
#include "tickpoketarget.h"
#include "workmanager.h"
#include "eventlog.h"

extern GlobalContext * global;

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

void TickPoke::startPoke(EventReceiver * pokee, std::string desc, int interval, int message) {
  targets.push_back(TickPokeTarget(pokee, interval, message));
  global->getEventLog()->log("TickPoke", "Registering " + desc + " as poke receiver with interval " + global->int2Str(interval) + "ms");
}

void TickPoke::stopPoke(EventReceiver * pokee, std::string desc, int message) {
  std::list<TickPokeTarget>::iterator it;
  for(it = targets.begin(); it != targets.end(); it++) {
    if (it->getPokee() == pokee && it->getMessage() == message) {
      targets.erase(it);
      global->getEventLog()->log("TickPoke", "Deregistering " + desc + " as poke receiver");
      return;
    }
  }
}
