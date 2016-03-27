#pragma once

#include "lock.h"

class ScopeLock {
public:
  ScopeLock(Lock & lock) : underlyinglock(lock), locked(true) {
    underlyinglock.lock();
  }
  ~ScopeLock() {
    if (locked) {
      unlock();
    }
  }
  void unlock() {
    locked = false;
    underlyinglock.unlock();
  }
  void lock() {
    underlyinglock.lock();
    locked = true;
  }
private:
  Lock & underlyinglock;
  bool locked;
};
