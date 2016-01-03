#pragma once

#include <map>
#include <string>

#include "threading.h"
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
  Thread<WorkManager> thread;
  BlockingQueue<Event> dataqueue;
  SignalEvents signalevents;
  Semaphore event;
  Semaphore readdata;
  DataBlockPool blockpool;
public:
  void init();
  void dispatchFDData(EventReceiver *, int);
  void dispatchFDData(EventReceiver *, int, char *, int);
  void dispatchTick(EventReceiver *, int);
  void dispatchEventNew(EventReceiver *, int);
  void dispatchEventConnected(EventReceiver *, int);
  void dispatchEventDisconnected(EventReceiver *, int);
  void dispatchEventSSLSuccess(EventReceiver *, int);
  void dispatchEventSSLFail(EventReceiver *, int);
  void dispatchEventFail(EventReceiver *, int, std::string);
  void dispatchEventSendComplete(EventReceiver *, int);
  void dispatchSignal(EventReceiver *, int);
  DataBlockPool * getBlockPool();
  bool overload() const;
  void run();
};
