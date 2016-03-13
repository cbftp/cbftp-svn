#include "asyncworker.h"

#include "workmanager.h"

AsyncWorker::AsyncWorker(WorkManager * wm, BlockingQueue<AsyncTask> & queue) :
  queue(queue), wm(wm)
{
}

void AsyncWorker::init() {
  thread.start("AsyncWorker", this);
}

void AsyncWorker::run() {
  while(1) {
    AsyncTask task = queue.pop();
    task.execute();
    wm->dispatchAsyncTaskComplete(task);
  }
}
