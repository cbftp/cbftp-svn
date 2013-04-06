#include "tickpoke.h"

TickPoke::TickPoke() {
  sem_init(&tick, 0, 0);
  pthread_create(&thread[0], global->getPthreadAttr(), runTicker, (void *) this);
  pthread_create(&thread[1], global->getPthreadAttr(), run, (void *) this);
  pthread_setname_np(thread[0], "Ticker");
  pthread_setname_np(thread[1], "TickWorker");

}

void TickPoke::runTickerInstance() {
  while(1) {
    usleep(SLEEPINTERVAL * 1000);
    sem_post(&tick);
  }
}

void TickPoke::runInstance() {
  std::list<TickPokeTarget>::iterator it;
  while(1) {
    sem_wait(&tick);
    for(it = targets.begin(); it != targets.end(); it++) {
      if (it->tick(SLEEPINTERVAL)) {
        it->getPokee()->tick(it->getMessage());
      }
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

void * TickPoke::runTicker(void * arg) {
  ((TickPoke *) arg)->runTickerInstance();
  return NULL;
}

void * TickPoke::run(void * arg) {
  ((TickPoke *) arg)->runInstance();
  return NULL;
}


