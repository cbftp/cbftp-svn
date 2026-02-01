#include "timereference.h"

#include <ctime>
#include <cstdio>
#include <sys/time.h>

#include "core/tickpoke.h"
#include "globalcontext.h"

#define INTERVAL 50

static int currentyear = 0;
static int currentmonth = 0;
static int currentday = 0;

TimeReference::TimeReference() : logtimestampms(false) {
  global->getTickPoke()->startPoke(this, "TimeReference", INTERVAL, 0);
}

void TimeReference::tick(int) {
  timeticker += INTERVAL;
}

std::string TimeReference::getCurrentTimeStamp(bool includedate) const {
  time_t rawtime = time(NULL);
  struct tm tm;
  localtime_r(&rawtime, &tm);
  char timebuf[32];
  int pos = 0;
  if (includedate) {
    pos = std::sprintf(timebuf, "%d-%.2d-%.2d ", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
  }
  pos += std::sprintf(timebuf + pos, "%.2d:%.2d:%.2d", tm.tm_hour, tm.tm_min, tm.tm_sec);
  if (logtimestampms) {
    struct timespec tp;
    clock_gettime(CLOCK_REALTIME, &tp);
    std::sprintf(timebuf + pos, ".%.3d", (unsigned int)(tp.tv_nsec / 1000000));
  }
  return timebuf;
}

std::string TimeReference::getCurrentFullTimeStamp() const {
  return getCurrentTimeStamp(true);
}

std::string TimeReference::getCurrentLogTimeStamp() const {
  return getCurrentTimeStamp(false);
}

bool TimeReference::getLogTimeStampMilliseconds() const {
  return logtimestampms;
}

void TimeReference::setLogTimeStampMilliseconds(bool ms) {
  logtimestampms = ms;
}

unsigned long long TimeReference::timeReference() const {
  return timeticker;
}

unsigned long long TimeReference::timePassedSince(unsigned long long timestamp) const {
  if (timestamp > timeticker) {
    return 0 - timestamp + timeticker;
  }
  return timeticker - timestamp;
}

void TimeReference::updateTime() {
  time_t rawtime;
  time(&rawtime);
  struct tm timedata;
  localtime_r(&rawtime, &timedata);
  currentyear = timedata.tm_year + 1900;
  currentmonth = timedata.tm_mon + 1;
  currentday = timedata.tm_mday;
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
