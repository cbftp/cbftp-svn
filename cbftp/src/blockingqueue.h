#pragma once

#include <list>

#include "semaphore.h"
#include "lock.h"
#include "scopelock.h"

template <class T> class BlockingQueue {
public:
  void push(T t) {
    ScopeLock lock(queuelock);
    queue.push_back(t);
    content.post();
  }
  T pop() {
    content.wait();
    ScopeLock lock(queuelock);
    T ret = queue.front();
    queue.pop_front();
    return ret;
  }
  unsigned int size() const {
    ScopeLock lock(queuelock);
    unsigned int size = queue.size();
    return size;
  }
private:
  std::list<T> queue;
  mutable Lock queuelock;
  Semaphore content;
};
