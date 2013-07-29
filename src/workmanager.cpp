#include "workmanager.h"

WorkManager::WorkManager() {
  sem_init(&dispatch, 0, 0);
  sem_init(&readdata, 0, 0);
  pthread_mutex_init(&dataqueue_mutex, NULL);
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
  pthread_setname_np(thread, "Worker");
}

void WorkManager::dispatchFDData(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, 0));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
  sem_wait(&readdata);
}

void WorkManager::dispatchFDData(EventReceiver * er, char * buf, int len) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, 1, buf, len));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchTick(EventReceiver * er, int interval) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, 2, interval));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchEventConnected(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, 3));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchEventDisconnected(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, 4));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchEventSSLSuccess(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, 5));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchEventSSLFail(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, 6));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

DataBlockPool * WorkManager::getBlockPool() {
  return &blockpool;
}

void WorkManager::runInstance() {
  char * data;
  while(1) {
    sem_wait(&dispatch);
    pthread_mutex_lock(&dataqueue_mutex);
    Event event = dataqueue.front();
    dataqueue.pop_front();
    pthread_mutex_unlock(&dataqueue_mutex);
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
      case 3:
        event.getReceiver()->FDConnected();
        break;
      case 4:
        event.getReceiver()->FDDisconnected();
        break;
      case 5:
        event.getReceiver()->FDSSLSuccess();
        break;
      case 6:
        event.getReceiver()->FDSSLFail();
        break;
    }
  }
}

void * WorkManager::run(void * arg) {
  ((WorkManager *) arg)->runInstance();
  return NULL;
}
