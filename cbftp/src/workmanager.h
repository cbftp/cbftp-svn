#pragma once

#include <map>
#include <string>

#include "threading.h"
#include "datablockpool.h"
#include "blockingqueue.h"
#include "signalevents.h"
#include "semaphore.h"
#include "pointer.h"
#include "event.h"

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
  void deferDelete(Pointer<EventReceiver>);
  DataBlockPool * getBlockPool();
  bool overload() const;
  void run();
};
