#pragma once

#include <pthread.h>
#include <unistd.h>

namespace Threading {
void setThreadName(pthread_t thread, const char * name);
void setCurrentThreadName(const char * name);
}

template <class T> class Thread {
public:
  void start(const char * name, T * instance) {
    this->instance = instance;
    pthread_create(&thread, NULL, run, (void *) this);
    Threading::setThreadName(thread, name);
  }
private:
  static void * run(void * target) {
    ((Thread *) target)->instance->run();
    return NULL;
  }
  T * instance;
  pthread_t thread;
};


