#include "workmanager.h"

WorkManager::WorkManager() {
  sem_init(&dispatch, 0, 0);
  sem_init(&readdata, 0, 0);
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
  pthread_setname_np(thread, "Worker");
}

void WorkManager::dispatchFDData(EventReceiver * er) {
  dataqueue.push_back(Event(er, 0));
  sem_post(&dispatch);
  sem_wait(&readdata);
}

void WorkManager::dispatchFDData(EventReceiver * er, char * buf, int len) {
  dataqueue.push_back(Event(er, 1, buf, len));
  sem_post(&dispatch);
}

void WorkManager::dispatchTick(EventReceiver * er, int interval) {
  dataqueue.push_back(Event(er, 2, interval));
  sem_post(&dispatch);
}

DataBlockPool * WorkManager::getBlockPool() {
  return &blockpool;
}

void WorkManager::runInstance() {
  char * data;
  while(1) {
    sem_wait(&dispatch);
    Event event = dataqueue.front();
    dataqueue.pop_front();
    switch (event.getType()) {
      case 0:
        event.getReceiver()->FDData();
        sem_post(&readdata);
        break;
      case 1:
        data = event.getData();
        event.getReceiver()->FDData(data, event.getDataLen());
        blockpool.returnBlock(data);
        break;
      case 2:
        event.getReceiver()->tick(event.getInterval());
        break;
    }
  }
}

void * WorkManager::run(void * arg) {
  ((WorkManager *) arg)->runInstance();
  return NULL;
}
