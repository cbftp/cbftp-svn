#pragma once

#include <pthread.h>
#include <semaphore.h>
#include <list>

#include "datablockpool.h"

#define MAXDATASIZE 2048
#define BUFSSIZE 32

class EventReceiver;
class Event;

class WorkManager {
private:
  char * bufs[BUFSSIZE];
  std::list<Event> dataqueue;
  pthread_t thread;
  static void * run(void *);
  sem_t dispatch;
  sem_t readdata;
  pthread_mutex_t dataqueue_mutex;
  DataBlockPool blockpool;
public:
  WorkManager();
  void dispatchFDData(EventReceiver *);
  void dispatchFDData(EventReceiver *, char *, int);
  void dispatchTick(EventReceiver *, int);
  void dispatchEventConnected(EventReceiver *);
  void dispatchEventDisconnected(EventReceiver *);
  void dispatchEventSSLSuccess(EventReceiver *);
  void dispatchEventSSLFail(EventReceiver *);
  DataBlockPool * getBlockPool();
  void runInstance();
};
