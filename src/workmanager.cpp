#include "workmanager.h"

#include "eventreceiver.h"
#include "scopelock.h"

#define OVERLOADSIZE 10
#define ASYNC_WORKERS 2

enum WorkType {
  WORK_DATA,
  WORK_DATABUF,
  WORK_TICK,
  WORK_CONNECTING,
  WORK_CONNECTED,
  WORK_DISCONNECTED,
  WORK_SSL_SUCCESS,
  WORK_SSL_FAIL,
  WORK_NEW,
  WORK_FAIL,
  WORK_SEND_COMPLETE,
  WORK_DELETE,
  WORK_ASYNC_COMPLETE,
  WORK_ASYNC_COMPLETE_P
};

void WorkManager::init() {
  thread.start("Worker", this);
  for (int i = 0; i < ASYNC_WORKERS; ++i) {
    asyncworkers.push_back(AsyncWorker(this, asyncqueue));
    asyncworkers.back().init();
  }
}

void WorkManager::dispatchFDData(EventReceiver * er, int sockid) {
  dataqueue.push(Event(er, WORK_DATA, sockid));
  event.post();
  readdata.wait();
}

void WorkManager::dispatchFDData(EventReceiver * er, int sockid, char * buf, int len) {
  dataqueue.push(Event(er, WORK_DATABUF, sockid, buf, len));
  event.post();
}

void WorkManager::dispatchTick(EventReceiver * er, int interval) {
  dataqueue.push(Event(er, WORK_TICK, interval));
  event.post();
}

void WorkManager::dispatchEventNew(EventReceiver * er, int sockid) {
  dataqueue.push(Event(er, WORK_NEW, sockid));
  event.post();
}

void WorkManager::dispatchEventConnecting(EventReceiver * er, int sockid, std::string addr) {
  dataqueue.push(Event(er, WORK_CONNECTING, sockid, addr));
  event.post();
}

void WorkManager::dispatchEventConnected(EventReceiver * er, int sockid) {
  dataqueue.push(Event(er, WORK_CONNECTED, sockid));
  event.post();
}

void WorkManager::dispatchEventDisconnected(EventReceiver * er, int sockid) {
  dataqueue.push(Event(er, WORK_DISCONNECTED, sockid));
  event.post();
}

void WorkManager::dispatchEventSSLSuccess(EventReceiver * er, int sockid) {
  dataqueue.push(Event(er, WORK_SSL_SUCCESS, sockid));
  event.post();
}

void WorkManager::dispatchEventSSLFail(EventReceiver * er, int sockid) {
  dataqueue.push(Event(er, WORK_SSL_FAIL, sockid));
  event.post();
}

void WorkManager::dispatchEventFail(EventReceiver * er, int sockid, std::string error) {
  dataqueue.push(Event(er, WORK_FAIL, sockid, error));
  event.post();
}

void WorkManager::dispatchEventSendComplete(EventReceiver * er, int sockid) {
  dataqueue.push(Event(er, WORK_SEND_COMPLETE, sockid));
  event.post();
}

void WorkManager::dispatchSignal(EventReceiver * er, int signal, int value) {
  if (signalevents.set(er, signal, value)) {
    event.post();
  }
}

void WorkManager::dispatchAsyncTaskComplete(AsyncTask & task) {
  if (task.dataIsPointer()) {
    dataqueue.push(Event(task.getReceiver(), WORK_ASYNC_COMPLETE_P, task.getType(), task.getData()));
  }
  else {
    dataqueue.push(Event(task.getReceiver(), WORK_ASYNC_COMPLETE, task.getType(), task.getNumData()));
  }
  event.post();
}

void WorkManager::deferDelete(Pointer<EventReceiver> er) {
  dataqueue.push(Event(er, WORK_DELETE));
  event.post();
}

void WorkManager::asyncTask(EventReceiver * er, int type, void (*taskfunction)(EventReceiver *, int), int data) {
  asyncqueue.push(AsyncTask(er, type, taskfunction, data));
}

void WorkManager::asyncTask(EventReceiver * er, int type, void (*taskfunction)(EventReceiver *, void *), void * data) {
  asyncqueue.push(AsyncTask(er, type, taskfunction, data));
}

DataBlockPool * WorkManager::getBlockPool() {
  return &blockpool;
}

bool WorkManager::overload() const {
  return dataqueue.size() >= OVERLOADSIZE;
}

void WorkManager::run() {
  while(1) {
    event.wait();
    if (signalevents.hasEvent()) {
      SignalData signal = signalevents.getClearFirst();
      signal.er->signal(signal.signal, signal.value);
    }
    else {
      Event event = dataqueue.pop();
      EventReceiver * er = event.getReceiver();
      int numdata = event.getNumericalData();
      switch (event.getType()) {
        case WORK_DATA:
          event.getReceiver()->FDData(numdata);
          readdata.post();
          break;
        case WORK_DATABUF: {
          char * data = static_cast<char *>(event.getData());
          er->FDData(numdata, data, event.getDataLen());
          blockpool.returnBlock(data);
          break;
        }
        case WORK_TICK:
          er->tick(numdata);
          break;
        case WORK_CONNECTING:
          er->FDConnecting(numdata, event.getStrData());
          break;
        case WORK_CONNECTED:
          er->FDConnected(numdata);
          break;
        case WORK_DISCONNECTED:
          er->FDDisconnected(numdata);
          break;
        case WORK_SSL_SUCCESS:
          er->FDSSLSuccess(numdata);
          break;
        case WORK_SSL_FAIL:
          er->FDSSLFail(numdata);
          break;
        case WORK_NEW:
          er->FDNew(numdata);
          break;
        case WORK_FAIL:
          er->FDFail(numdata, event.getStrData());
          break;
        case WORK_SEND_COMPLETE:
          er->FDSendComplete(numdata);
          break;
        case WORK_DELETE: // will be deleted when going out of scope
          break;
        case WORK_ASYNC_COMPLETE:
          er->asyncTaskComplete(numdata, event.getNumericalData2());
          break;
        case WORK_ASYNC_COMPLETE_P:
          er->asyncTaskComplete(numdata, event.getData());
          break;
      }
    }
  }
}
