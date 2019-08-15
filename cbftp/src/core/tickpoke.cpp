#include "tickpoke.h"

#include <unistd.h>

#include "workmanager.h"
#include "tickpoketarget.h"

namespace Core {

#define DEFAULTSLEEPINTERVAL 50

TickPoke::TickPoke(WorkManager& wm) : wm(wm), forever(true), sleeptime(DEFAULTSLEEPINTERVAL) {
}

TickPoke::~TickPoke() {
  breakLoop();
}

void TickPoke::tickerLoop() {
  std::lock_guard<std::recursive_mutex> lock(looplock);
  while (forever) {
    wm.dispatchTick(this, sleeptime);
    usleep(sleeptime * 1000);
  }
}

void TickPoke::tickerThread(const std::string& prefix, int id) {
  thread.start((prefix + "-tp-" + std::to_string(id)).c_str(), this);
}

void TickPoke::stop() {
  breakLoop();
  thread.join();
}

void TickPoke::run() {
  tickerLoop();
}

void TickPoke::breakLoop() {
  forever = false;
  std::lock_guard<std::recursive_mutex> lock(looplock); // grab lock to make sure the loop has stopped running
}

void TickPoke::tick(int interval) {
  if (!forever)
  {
    return;
  }
  std::list<std::pair<EventReceiver*, int>> ticklist;
  for (auto& target : targets) {
    EventReceiver* er = target.getPokee();
    int ticks = target.tick(interval);
    for (int i = 0; i < ticks; ++i) {
      ticklist.emplace_back(er, target.getMessage());
    }
  }
  for (auto& tickitem : ticklist) {
    tickitem.first->tick(tickitem.second);
  }
}

void TickPoke::startPoke(EventReceiver* pokee, const std::string& desc, int interval, int message) {
  targets.emplace_back(pokee, interval, message, desc);
}

void TickPoke::stopPoke(const EventReceiver* pokee, int message) {
  std::list<TickPokeTarget>::iterator it;
  for (it = targets.begin(); it != targets.end(); ++it) {
    if (it->getPokee() == pokee && it->getMessage() == message) {
      targets.erase(it);
      return;
    }
  }
}

void TickPoke::setGranularity(unsigned int interval) {
    sleeptime = interval;
}

} // namespace Core
