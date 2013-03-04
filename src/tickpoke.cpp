#include "tickpoke.h"

TickPoke::TickPoke() {
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
  pthread_setname_np(thread, "TickPokeThread");
}

void TickPoke::runInstance() {
  std::list<TickPokeTarget>::iterator it;
  while(1) {
    usleep(SLEEPINTERVAL * 1000);
    for(it = targets.begin(); it != targets.end(); it++) {
      if (it->tick(SLEEPINTERVAL)) {
        pokes[it->getPokee()].push_back(it->getMessage());
        sem_post(it->getPokee());
      }
    }
  }
}

void TickPoke::startPoke(sem_t * pokee, int interval, int message) {
  if (pokes.find(pokee) == pokes.end()) {
    pokes[pokee] = std::list<int>();
  }
  targets.push_back(TickPokeTarget(pokee, interval, message));
}

void TickPoke::stopPoke(sem_t * pokee, int message) {
  std::list<TickPokeTarget>::iterator it;
  for(it = targets.begin(); it != targets.end(); it++) {
    if (it->getPokee() == pokee && it->getMessage() == message) {
      targets.erase(it);
      return;
    }
  }

}

int TickPoke::getMessage(sem_t * pokee) {
  int message = pokes[pokee].front();
  pokes[pokee].pop_front();
  return message;
}

bool TickPoke::isPoked(sem_t * pokee) {
  return pokes[pokee].size() > 0;
}

void * TickPoke::run(void * arg) {
  ((TickPoke *) arg)->runInstance();
  return NULL;
}
