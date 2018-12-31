#include "signalevents.h"

#include <cassert>

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
  else if (!slot6.set) {
    slot6.set = true;
    slot6.er = er;
    slot6.signal = signal;
    slot6.value = value;
  }
  else if (slot6.er == er && slot6.signal == signal && slot6.value == value) {
    return false;
  }
  else if (!slot7.set) {
    slot7.set = true;
    slot7.er = er;
    slot7.signal = signal;
    slot7.value = value;
  }
  else if (slot7.er == er && slot7.signal == signal && slot7.value == value) {
    return false;
  }
  else if (!slot8.set) {
    slot8.set = true;
    slot8.er = er;
    slot8.signal = signal;
    slot8.value = value;
  }
  else if (slot8.er == er && slot8.signal == signal && slot8.value == value) {
    return false;
  }
  else if (!slot9.set) {
    slot9.set = true;
    slot9.er = er;
    slot9.signal = signal;
    slot9.value = value;
  }
  else if (slot9.er == er && slot9.signal == signal && slot9.value == value) {
    return false;
  }
  else if (!slot10.set) {
    slot10.set = true;
    slot10.er = er;
    slot10.signal = signal;
    slot10.value = value;
  }
  else if (slot10.er == er && slot10.signal == signal && slot10.value == value) {
    return false;
  }
  else {
    assert(false);
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
  else if (slot6.set) {
    ret = slot6;
    slot6.set = false;
  }
  else if (slot7.set) {
    ret = slot7;
    slot7.set = false;
  }
  else if (slot8.set) {
    ret = slot8;
    slot8.set = false;
  }
  else if (slot9.set) {
    ret = slot9;
    slot9.set = false;
  }
  else if (slot10.set) {
    ret = slot10;
    slot10.set = false;
  }
  else {
    assert(false);
  }
  hasevent--;
  return ret;
}
