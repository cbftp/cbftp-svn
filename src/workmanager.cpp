#include "workmanager.h"

#include "eventreceiver.h"
#include "globalcontext.h"
#include "event.h"

extern GlobalContext * global;

WorkManager::WorkManager() {
  sem_init(&readdata, 0, 0);
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, "Worker");
#endif
}

void WorkManager::dispatchFDData(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_DATA));
  sem_wait(&readdata);
}

void WorkManager::dispatchFDData(EventReceiver * er, char * buf, int len) {
  dataqueue.push(Event(er, WORK_DATABUF, buf, len));
}

void WorkManager::dispatchTick(EventReceiver * er, int interval) {
  dataqueue.push(Event(er, WORK_TICK, interval));
}

void WorkManager::dispatchEventNew(EventReceiver * er, int sockfd) {
  dataqueue.push(Event(er, WORK_NEW, sockfd));
}

void WorkManager::dispatchEventConnected(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_CONNECTED));
}

void WorkManager::dispatchEventDisconnected(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_DISCONNECTED));
}

void WorkManager::dispatchEventSSLSuccess(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_SSL_SUCCESS));
}

void WorkManager::dispatchEventSSLFail(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_SSL_FAIL));
}

DataBlockPool * WorkManager::getBlockPool() {
  return &blockpool;
}

bool WorkManager::overload() {
  return dataqueue.size() >= 10;
}

void WorkManager::runInstance() {
  char * data;
  while(1) {
    Event event = dataqueue.pop();
    EventReceiver * er = event.getReceiver();
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
      case WORK_NEW:
        er->FDNew(event.getInterval());
        break;
    }
  }
}

void * WorkManager::run(void * arg) {
  ((WorkManager *) arg)->runInstance();
  return NULL;
}
