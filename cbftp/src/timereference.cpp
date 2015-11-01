#include "timereference.h"

#include "globalcontext.h"
#include "tickpoke.h"

extern GlobalContext * global;

TimeReference::TimeReference() {
  global->getTickPoke()->startPoke(this, "TimeReference", INTERVAL, 0);
}

void TimeReference::tick(int) {
  timeticker += INTERVAL;
}

unsigned long long TimeReference::timeReference() {
  return timeticker;
}

unsigned long long TimeReference::timePassedSince(unsigned long long timestamp) {
  if (timestamp > timeticker) {
    return 0 - timestamp + timeticker;
  }
  return timeticker - timestamp;
}
