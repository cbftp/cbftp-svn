#pragma once

#include <semaphore.h>
#include "eventreceiver.h"

class TickPokeTarget {
private:
  EventReceiver * pokee;
  int interval;
  int currentval;
  int message;
public:
  TickPokeTarget(EventReceiver *, int, int);
  int getMessage();
  EventReceiver * getPokee();
  bool tick(int);
};
