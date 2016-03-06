#include "signalevents.h"

#include "util.h"
#include "scopelock.h"

SignalEvents::SignalEvents() :
  hasevent(0), signallock(true) // the lock needs to be recursive since the set
{                               // method is supposed to be called from signal
                                // handlers, which may interrupt current
}                               // execution at any time

bool SignalEvents::set(EventReceiver * er, int signal, int value) {
  ScopeLock lock(signallock);
  if (!slot1.set) {
    slot1.set = true;
    slot1.er = er;
    slot1.signal = signal;
    slot1.value = value;
  }
  else if (slot1.er == er && slot1.signal == signal && slot1.value == value) {
    return false;
  }
  else if (!slot2.set) {
    slot2.set = true;
    slot2.er = er;
    slot2.signal = signal;
    slot2.value = value;
  }
  else if (slot2.er == er && slot2.signal == signal && slot2.value == value) {
    return false;
  }
  else if (!slot3.set) {
    slot3.set = true;
    slot3.er = er;
    slot3.signal = signal;
    slot3.value = value;
  }
  else if (slot3.er == er && slot3.signal == signal && slot3.value == value) {
    return false;
  }
  else if (!slot4.set) {
    slot4.set = true;
    slot4.er = er;
    slot4.signal = signal;
    slot4.value = value;
  }
  else if (slot4.er == er && slot4.signal == signal && slot4.value == value) {
    return false;
  }
  else if (!slot5.set) {
    slot5.set = true;
    slot5.er = er;
    slot5.signal = signal;
    slot5.value = value;
  }
  else if (slot5.er == er && slot5.signal == signal && slot5.value == value) {
    return false;
  }
  else {
    util::assert(false);
  }
  hasevent++;
  return true;
}

bool SignalEvents::hasEvent() const {
  ScopeLock lock(signallock);
  return hasevent;
}

SignalData SignalEvents::getClearFirst() {
  ScopeLock lock(signallock);
  SignalData ret;
  if (slot1.set) {
    ret = slot1;
    slot1.set = false;
  }
  else if (slot2.set) {
    ret = slot2;
    slot2.set = false;
  }
  else if (slot3.set) {
    ret = slot3;
    slot3.set = false;
  }
  else if (slot4.set) {
    ret = slot4;
    slot4.set = false;
  }
  else if (slot5.set) {
    ret = slot5;
    slot5.set = false;
  }
  else {
    util::assert(false);
  }
  hasevent--;
  return ret;
}
