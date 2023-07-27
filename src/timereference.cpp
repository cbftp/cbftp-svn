#include "timereference.h"

#include <ctime>
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

std::string TimeReference::getCurrentLogTimeStamp() const {
  struct timeval tp;
  gettimeofday(&tp, nullptr);
  time_t rawtime = time(NULL);
  char timebuf[26];
  ctime_r(&rawtime, timebuf);
  if (logtimestampms) {
    std::string ms = std::to_string(tp.tv_usec / 1000);
    while (ms.length() < 3) {
      ms = "0" + ms;
    }
    return std::string(timebuf + 11, 8) + "." + ms;
  }
  return std::string(timebuf + 11, 8);
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
