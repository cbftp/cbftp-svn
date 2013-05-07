#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <list>

#include "eventreceiver.h"
#include "globalcontext.h"
#include "event.h"
#include "datablockpool.h"

#define MAXDATASIZE 2048
#define BUFSSIZE 32
extern GlobalContext * global;

class WorkManager {
private:
  char * bufs[BUFSSIZE];
  std::list<Event> dataqueue;
  pthread_t thread;
  static void * run(void *);
  sem_t dispatch;
  sem_t readdata;
  DataBlockPool blockpool;
public:
  WorkManager();
  void dispatchFDData(EventReceiver *);
  void dispatchFDData(EventReceiver *, char *, int);
  void dispatchTick(EventReceiver *, int);
  DataBlockPool * getBlockPool();
  void runInstance();
};
