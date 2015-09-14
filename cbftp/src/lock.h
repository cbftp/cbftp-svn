#pragma once

#include <pthread.h>

class Lock {
public:
  Lock() {
    pthread_mutex_init(&mutex, NULL);
  }
  Lock(bool recursive) {
    if (recursive) {
      pthread_mutexattr_t attr;
      pthread_mutexattr_init(&attr);
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      pthread_mutex_init(&mutex, &attr);
    }
    else {
      pthread_mutex_init(&mutex, NULL);
    }
  }
  ~Lock() {
    pthread_mutex_destroy(&mutex);
  }
  void lock() {
    pthread_mutex_lock(&mutex);
  }
  void unlock() {
    pthread_mutex_unlock(&mutex);
  }
private:
  pthread_mutex_t mutex;
};
