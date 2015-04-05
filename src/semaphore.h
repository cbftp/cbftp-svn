#pragma once

#ifndef __APPLE__

#include <semaphore.h>

class Semaphore {
public:
  Semaphore() {
    sem_init(&semaphore, 0, 0);
  }
  ~Semaphore() {
    sem_destroy(&semaphore);
  }
  void post() {
    sem_post(&semaphore);
  }
  void wait() {
    sem_wait(&semaphore);
  }
private:
  sem_t semaphore;
};

#else

#include <dispatch/dispatch.h>

class Semaphore {
public:
  Semaphore() :
    semaphore(dispatch_semaphore_create(0)) {
  }
  ~Semaphore() {
    dispatch_release(semaphore);
  }
  void post() {
    dispatch_semaphore_signal(semaphore);
  }
  void wait() {
    dispatch_semaphore_wait(semaphore, DISPATCH_TIME_FOREVER);
  }
private:
  dispatch_semaphore_t semaphore;
};

#endif
