#include "tickpoke.h"

#include <unistd.h>

#include "tickpoketarget.h"
#include "workmanager.h"

TickPoke::TickPoke(WorkManager * wm) :
  wm(wm),
  forever(true)
{

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
  std::list<std::pair<EventReceiver *, int> > ticklist;
  for(it = targets.begin(); it != targets.end(); it++) {
    if (it->tick(interval)) {
      EventReceiver * er = it->getPokee();
      ticklist.push_back(std::pair<EventReceiver *, int>(er, it->getMessage()));
    }
  }
  for (std::list<std::pair<EventReceiver *, int> >::iterator it = ticklist.begin(); it != ticklist.end(); it++) {
    it->first->tick(it->second);
  }
}

void TickPoke::startPoke(EventReceiver * pokee, std::string desc, int interval, int message) {
  targets.push_back(TickPokeTarget(pokee, interval, message, desc));
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
