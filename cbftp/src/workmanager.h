#pragma once

#include <pthread.h>
#include <map>
#include <string>

#include "datablockpool.h"
#include "blockingqueue.h"
#include "signalevents.h"
#include "semaphore.h"
#include "event.h"

#define MAXDATASIZE 2048
#define BUFSSIZE 32
#define OVERLOADSIZE 10

enum WorkType {
  WORK_DATA,
  WORK_DATABUF,
  WORK_TICK,
  WORK_CONNECTED,
  WORK_DISCONNECTED,
  WORK_SSL_SUCCESS,
  WORK_SSL_FAIL,
  WORK_NEW,
  WORK_FAIL,
  WORK_SEND_COMPLETE,
  WORK_CHILD
};

class EventReceiver;

class WorkManager {
private:
  BlockingQueue<Event> dataqueue;
  SignalEvents signalevents;
  Semaphore event;
  pthread_t thread;
  static void * run(void *);
  Semaphore readdata;
  DataBlockPool blockpool;
public:
  void init();
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
  void dispatchSignal(EventReceiver *, int);
  DataBlockPool * getBlockPool();
  bool overload() const;
  void runInstance();
};
