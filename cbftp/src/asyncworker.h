#pragma once

#include "threading.h"
#include "blockingqueue.h"
#include "asynctask.h"

class WorkManager;

class AsyncWorker {
public:
  AsyncWorker(WorkManager *, BlockingQueue<AsyncTask> &);
  void init();
  void run();
private:
  Thread<AsyncWorker> thread;
  BlockingQueue<AsyncTask> & queue;
  WorkManager * wm;
};
