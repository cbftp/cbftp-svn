#pragma once

#include "lock.h"

class EventReceiver;

struct SignalData {
  SignalData() : set(false), signal(0), value(0), er(NULL) { }
  bool set;
  int signal;
  int value;
  EventReceiver * er;
};

class SignalEvents {
public:
  SignalEvents();
  bool set(EventReceiver *, int, int);
  bool hasEvent() const;
  SignalData getClearFirst();
private:
  int hasevent;
  SignalData slot1;
  SignalData slot2;
  SignalData slot3;
  SignalData slot4;
  SignalData slot5;
  SignalData slot6;
  SignalData slot7;
  SignalData slot8;
  SignalData slot9;
  SignalData slot10;
  mutable Lock signallock;
};
