#include "timereference.h"

#include "globalcontext.h"
#include "tickpoke.h"

extern GlobalContext * global;

TimeReference::TimeReference() {
  global->getTickPoke()->startPoke(this, "TimeReference", INTERVAL, 0);
}

void TimeReference::tick(int) {
  time += INTERVAL;
}

unsigned long long TimeReference::timeReference() {
  return time;
}

unsigned long long TimeReference::timePassedSince(unsigned long long timestamp) {
  if (timestamp > time) {
    return 0 - timestamp + time;
  }
  return time - timestamp;
}
