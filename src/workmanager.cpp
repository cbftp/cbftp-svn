#include "workmanager.h"

#include "eventreceiver.h"
#include "globalcontext.h"
#include "event.h"

extern GlobalContext * global;

WorkManager::WorkManager() {
  pthread_create(&thread, global->getPthreadAttr(), run, (void *) this);
#ifdef _ISOC95_SOURCE
  pthread_setname_np(thread, "Worker");
#endif
}

void WorkManager::dispatchFDData(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_DATA));
  event.post();
  readdata.wait();
}

void WorkManager::dispatchFDData(EventReceiver * er, char * buf, int len) {
  dataqueue.push(Event(er, WORK_DATABUF, buf, len));
  event.post();
}

void WorkManager::dispatchTick(EventReceiver * er, int interval) {
  dataqueue.push(Event(er, WORK_TICK, interval));
  event.post();
}

void WorkManager::dispatchEventNew(EventReceiver * er, int sockfd) {
  dataqueue.push(Event(er, WORK_NEW, sockfd));
  event.post();
}

void WorkManager::dispatchEventConnected(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_CONNECTED));
  event.post();
}

void WorkManager::dispatchEventDisconnected(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_DISCONNECTED));
  event.post();
}

void WorkManager::dispatchEventSSLSuccess(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_SSL_SUCCESS));
  event.post();
}

void WorkManager::dispatchEventSSLFail(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_SSL_FAIL));
  event.post();
}

void WorkManager::dispatchEventFail(EventReceiver * er, std::string error) {
  dataqueue.push(Event(er, WORK_FAIL, error));
  event.post();
}

void WorkManager::dispatchEventSendComplete(EventReceiver * er) {
  dataqueue.push(Event(er, WORK_SEND_COMPLETE));
  event.post();
}

void WorkManager::dispatchSignal(EventReceiver * er, int signal) {
  if (signalevents.set(er, signal)) {
    event.post();
  }
}

DataBlockPool * WorkManager::getBlockPool() {
  return &blockpool;
}

bool WorkManager::overload() const {
  return dataqueue.size() >= 10;
}

void WorkManager::runInstance() {
  char * data;
  while(1) {
    event.wait();
    if (signalevents.hasEvent()) {
      SignalData signal = signalevents.getClearFirst();
      signal.er->signal(signal.signal);
    }
    else {
      Event event = dataqueue.pop();
      EventReceiver * er = event.getReceiver();
      switch (event.getType()) {
        case WORK_DATA:
          event.getReceiver()->FDData();
          readdata.post();
          break;
        case WORK_DATABUF:
          data = event.getData();
          er->FDData(data, event.getDataLen());
          blockpool.returnBlock(data);
          break;
        case WORK_TICK:
          er->tick(event.getNumericalData());
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
          er->FDNew(event.getNumericalData());
          break;
        case WORK_FAIL:
          er->FDFail(event.getStrData());
          break;
        case WORK_SEND_COMPLETE:
          er->FDSendComplete();
          break;
      }
    }
  }
}

void * WorkManager::run(void * arg) {
  ((WorkManager *) arg)->runInstance();
  return NULL;
}
