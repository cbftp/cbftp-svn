#pragma once

#include "core/eventreceiver.h"

class TimeReference : public Core::EventReceiver {
public:
  TimeReference();
  void tick(int);
  unsigned long long timeReference() const;
  unsigned long long timePassedSince(unsigned long long) const;
  std::string getCurrentLogTimeStamp() const;
  bool getLogTimeStampMilliseconds() const;
  void setLogTimeStampMilliseconds(bool ms);
private:
  unsigned long long timeticker;
  bool logtimestampms;

public:
  static void updateTime();
  static int currentYear();
  static int currentMonth();
  static int currentDay();
};
