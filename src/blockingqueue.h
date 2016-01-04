#pragma once

#include <list>

#include "semaphore.h"
#include "lock.h"
#include "scopelock.h"

/*
 * pop/lock/begin/end/erase MUST be used by the same thread.
 * begin/end/erase may only be called while queuelock is acquired.
 * push/size can be used from any thread.
 */
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
  Lock & lock() {
    return queuelock;
  }
  typename std::list<T>::iterator begin() {
    return queue.begin();
  }
  typename std::list<T>::iterator end() {
    return queue.end();
  }
  void erase(typename std::list<T>::iterator it) {
    queue.erase(it);
    content.wait();
  }
private:
  std::list<T> queue;
  mutable Lock queuelock;
  Semaphore content;
};
