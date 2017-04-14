#pragma once

#include <map>
#include <string>
#include <list>
#include <vector>

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
#include "prio.h"

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
  std::vector<BlockingQueue<Event> *> eventqueues;
public:
  WorkManager();
  void init();
  void dispatchFDData(EventReceiver *, int);
  bool dispatchFDData(EventReceiver *, int, char *, int);
  bool dispatchFDData(EventReceiver *, int, char *, int, Prio);
  void dispatchTick(EventReceiver *, int);
  void dispatchEventNew(EventReceiver *, int);
  void dispatchEventNew(EventReceiver *, int, Prio);
  void dispatchEventConnecting(EventReceiver *, int, const std::string &);
  void dispatchEventConnecting(EventReceiver *, int, const std::string &, Prio);
  void dispatchEventConnected(EventReceiver *, int);
  void dispatchEventConnected(EventReceiver *, int, Prio);
  void dispatchEventDisconnected(EventReceiver *, int);
  void dispatchEventDisconnected(EventReceiver *, int, Prio);
  void dispatchEventSSLSuccess(EventReceiver *, int, const std::string &);
  void dispatchEventSSLSuccess(EventReceiver *, int, const std::string &, Prio);
  void dispatchEventSSLFail(EventReceiver *, int);
  void dispatchEventSSLFail(EventReceiver *, int, Prio);
  void dispatchEventFail(EventReceiver *, int, const std::string &);
  void dispatchEventFail(EventReceiver *, int, const std::string &, Prio);
  void dispatchEventSendComplete(EventReceiver *, int);
  void dispatchEventSendComplete(EventReceiver *, int, Prio);
  void dispatchSignal(EventReceiver *, int, int);
  void dispatchAsyncTaskComplete(AsyncTask &);
  void deferDelete(Pointer<EventReceiver>);
  void asyncTask(EventReceiver *, int, void (*)(EventReceiver *, int), int);
  void asyncTask(EventReceiver *, int, void (*)(EventReceiver *, void *), void *);
  DataBlockPool * getBlockPool();
  bool overload();
  bool lowPrioOverload();
  void run();
  void addReadyNotify(EventReceiver *);
};
