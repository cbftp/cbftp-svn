#pragma once

#include <string>

class EventReceiver;

class TickPokeTarget {
private:
  EventReceiver * pokee;
  int interval;
  int currentval;
  int message;
  std::string desc;
public:
  TickPokeTarget(EventReceiver *, int, int, std::string);
  int getMessage() const;
  EventReceiver * getPokee() const;
  bool tick(int);
};
