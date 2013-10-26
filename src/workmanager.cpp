#include "workmanager.h"

#include "eventreceiver.h"
#include "globalcontext.h"
#include "event.h"

extern GlobalContext * global;

WorkManager::WorkManager() {
  sem_init(&dispatch, 0, 0);
  sem_init(&readdata, 0, 0);
  pthread_mutex_init(&dataqueue_mutex, NULL);
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, "Worker");
#endif
}

void WorkManager::dispatchFDData(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, WORK_DATA));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
  sem_wait(&readdata);
}

void WorkManager::dispatchFDData(EventReceiver * er, char * buf, int len) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, WORK_DATABUF, buf, len));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchTick(EventReceiver * er, int interval) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, WORK_TICK, interval));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchEventConnected(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, WORK_CONNECTED));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchEventDisconnected(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, WORK_DISCONNECTED));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchEventSSLSuccess(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, WORK_SSL_SUCCESS));
  pthread_mutex_unlock(&dataqueue_mutex);
  sem_post(&dispatch);
}

void WorkManager::dispatchEventSSLFail(EventReceiver * er) {
  pthread_mutex_lock(&dataqueue_mutex);
  dataqueue.push_back(Event(er, WORK_SSL_FAIL));
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
    EventReceiver * er = event.getReceiver();
    er->lock();
    switch (event.getType()) {
      case WORK_DATA:
        event.getReceiver()->FDData();
        sem_post(&readdata);
        break;
      case WORK_DATABUF:
        data = event.getData();
        er->FDData(data, event.getDataLen());
        blockpool.returnBlock(data);
        break;
      case WORK_TICK:
        er->tick(event.getInterval());
        break;
      case WORK_CONNECTED:
        er->FDConnected();
        break;
      case WORK_DISCONNECTED:
        er->FDDisconnected();
        break;
      case WORK_SSL_SUCCESS:
        er->FDSSLSuccess();
        break;
      case WORK_SSL_FAIL:
        er->FDSSLFail();
        break;
    }
    er->unlock();
  }
}

void * WorkManager::run(void * arg) {
  ((WorkManager *) arg)->runInstance();
  return NULL;
}
