#pragma once

#include <map>
#include <string>
#include <list>

#include "threading.h"
#include "datablockpool.h"
#include "blockingqueue.h"
#include "signalevents.h"
#include "semaphore.h"
#include "lock.h"
#include "pointer.h"
#include "event.h"
#include "asyncworker.h"
#include "asynctask.h"

class EventReceiver;

class WorkManager {
private:
  Thread<WorkManager> thread;
  std::list<AsyncWorker> asyncworkers;
  BlockingQueue<Event> dataqueue;
  BlockingQueue<Event> highprioqueue;
  BlockingQueue<Event> lowprioqueue;
  BlockingQueue<AsyncTask> asyncqueue;
  Lock readylock;
  std::list<EventReceiver *> readynotify;
  SignalEvents signalevents;
  Semaphore event;
  Semaphore readdata;
  DataBlockPool blockpool;
  bool overloaded;
  bool lowpriooverloaded;
public:
  WorkManager();
  void init();
  void dispatchFDData(EventReceiver *, int);
  bool dispatchFDData(EventReceiver *, int, char *, int);
  bool dispatchLowPrioFDData(EventReceiver *, int, char *, int);
  void dispatchTick(EventReceiver *, int);
  void dispatchEventNew(EventReceiver *, int);
  void dispatchEventConnecting(EventReceiver *, int, std::string);
  void dispatchEventConnected(EventReceiver *, int);
  void dispatchEventDisconnected(EventReceiver *, int);
  void dispatchEventSSLSuccess(EventReceiver *, int);
  void dispatchEventSSLFail(EventReceiver *, int);
  void dispatchEventFail(EventReceiver *, int, std::string);
  void dispatchEventSendComplete(EventReceiver *, int);
  void dispatchSignal(EventReceiver *, int, int);
  void dispatchAsyncTaskComplete(AsyncTask &);
  void deferDelete(Pointer<EventReceiver>);
  void asyncTask(EventReceiver *, int, void (*)(EventReceiver *, int), int);
  void asyncTask(EventReceiver *, int, void (*)(EventReceiver *, void *), void *);
  DataBlockPool * getBlockPool();
  bool overload();
  void run();
  void addReadyNotify(EventReceiver *);
};
