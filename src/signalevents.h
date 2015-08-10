#pragma once

#include "lock.h"

class EventReceiver;

struct SignalData {
  SignalData() : set(false), signal(0), er(NULL) { }
  bool set;
  int signal;
  EventReceiver * er;
};

class SignalEvents {
public:
  SignalEvents();
  bool set(EventReceiver * er, int signal);
  bool hasEvent() const;
  SignalData getClearFirst();
private:
  int hasevent;
  SignalData slot1;
  SignalData slot2;
  SignalData slot3;
  SignalData slot4;
  SignalData slot5;
  mutable Lock signallock;
};
