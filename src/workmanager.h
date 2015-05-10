#pragma once

#include <pthread.h>
#include <list>
#include <string>

#include "datablockpool.h"
#include "blockingqueue.h"
#include "semaphore.h"

#define MAXDATASIZE 2048
#define BUFSSIZE 32
#define OVERLOADSIZE 10

#define WORK_DATA 2534
#define WORK_DATABUF 2535
#define WORK_TICK 2536
#define WORK_CONNECTED 2537
#define WORK_DISCONNECTED 2538
#define WORK_SSL_SUCCESS 2539
#define WORK_SSL_FAIL 2540
#define WORK_NEW 2541
#define WORK_FAIL 2542
#define WORK_SEND_COMPLETE 2543

class EventReceiver;
class Event;

class WorkManager {
private:
  BlockingQueue<Event> dataqueue;
  pthread_t thread;
  static void * run(void *);
  Semaphore readdata;
  DataBlockPool blockpool;
public:
  WorkManager();
  void dispatchFDData(EventReceiver *);
  void dispatchFDData(EventReceiver *, char *, int);
  void dispatchTick(EventReceiver *, int);
  void dispatchEventNew(EventReceiver *, int);
  void dispatchEventConnected(EventReceiver *);
  void dispatchEventDisconnected(EventReceiver *);
  void dispatchEventSSLSuccess(EventReceiver *);
  void dispatchEventSSLFail(EventReceiver *);
  void dispatchEventFail(EventReceiver *, std::string);
  void dispatchEventSendComplete(EventReceiver *);
  DataBlockPool * getBlockPool();
  bool overload() const;
  void runInstance();
};
