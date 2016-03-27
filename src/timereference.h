#pragma once

#include "core/eventreceiver.h"

class TimeReference : public EventReceiver {
public:
  TimeReference();
  void tick(int);
  unsigned long long timeReference();
  unsigned long long timePassedSince(unsigned long long);
private:
  unsigned long long timeticker;

public:
  static void updateTime();
  static int currentYear();
  static int currentMonth();
  static int currentDay();
};
