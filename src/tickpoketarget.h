#pragma once

class EventReceiver;

class TickPokeTarget {
private:
  EventReceiver * pokee;
  int interval;
  int currentval;
  int message;
public:
  TickPokeTarget(EventReceiver *, int, int);
  int getMessage() const;
  EventReceiver * getPokee() const;
  bool tick(int);
};
