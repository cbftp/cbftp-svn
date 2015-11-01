#pragma once

#include "eventreceiver.h"

#define INTERVAL 50

class TimeReference : public EventReceiver {
public:
  TimeReference();
  void tick(int);
  unsigned long long timeReference();
  unsigned long long timePassedSince(unsigned long long);
private:
  unsigned long long timeticker;
};
