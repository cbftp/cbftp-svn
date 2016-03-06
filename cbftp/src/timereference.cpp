#include "timereference.h"

#include <time.h>

#include "globalcontext.h"
#include "tickpoke.h"

extern GlobalContext * global;

static int currentyear = 0;
static int currentmonth = 0;
static int currentday = 0;

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

void TimeReference::updateTime() {
  time_t rawtime;
  time(&rawtime);
  struct tm * timedata = localtime(&rawtime);
  currentyear = timedata->tm_year + 1900;
  currentmonth = timedata->tm_mon + 1;
  currentday = timedata->tm_mday;
}

int TimeReference::currentYear() {
  return currentyear;
}

int TimeReference::currentMonth() {
  return currentmonth;
}

int TimeReference::currentDay() {
  return currentday;
}
