#include "workmanager.h"

#include "eventreceiver.h"
#include "scopelock.h"

#define READYSIZE 8
#define OVERLOADSIZE 32
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

WorkManager::WorkManager() : overloaded(false), lowpriooverloaded(false) {
  for (int i = 0; i < ASYNC_WORKERS; ++i) {
    asyncworkers.push_back(AsyncWorker(this, asyncqueue));
  }
  eventqueues.push_back(&lowprioqueue);
  eventqueues.push_back(&dataqueue);
  eventqueues.push_back(&highprioqueue);
}

void WorkManager::init() {
  thread.start("Worker", this);
  std::list<AsyncWorker>::iterator it;
  for (it = asyncworkers.begin(); it != asyncworkers.end(); it++) {
    it->init();
  }
}

void WorkManager::dispatchFDData(EventReceiver * er, int sockid) {
  highprioqueue.push(Event(er, WORK_DATA, sockid));
  event.post();
  readdata.wait();
}

bool WorkManager::dispatchFDData(EventReceiver * er, int sockid, char * buf, int len) {
  return dispatchFDData(er, sockid, buf, len, PRIO_NORMAL);
}

bool WorkManager::dispatchFDData(EventReceiver * er, int sockid, char * buf, int len, Prio prio) {
  eventqueues[prio]->push(Event(er, WORK_DATABUF, sockid, buf, len));
  event.post();
  if (prio == PRIO_LOW) {
    return !lowPrioOverload();
  }
  return !overload();
}

void WorkManager::dispatchTick(EventReceiver * er, int interval) {
  highprioqueue.push(Event(er, WORK_TICK, interval));
  event.post();
}

void WorkManager::dispatchEventNew(EventReceiver * er, int sockid) {
  dispatchEventNew(er, sockid, PRIO_NORMAL);
}

void WorkManager::dispatchEventNew(EventReceiver * er, int sockid, Prio prio) {
  eventqueues[prio]->push(Event(er, WORK_NEW, sockid));
  event.post();
}

void WorkManager::dispatchEventConnecting(EventReceiver * er, int sockid, const std::string & addr) {
  dispatchEventConnecting(er, sockid, addr, PRIO_NORMAL);
}

void WorkManager::dispatchEventConnecting(EventReceiver * er, int sockid, const std::string & addr, Prio prio) {
  eventqueues[prio]->push(Event(er, WORK_CONNECTING, sockid, addr));
  event.post();
}

void WorkManager::dispatchEventConnected(EventReceiver * er, int sockid) {
  dispatchEventConnected(er, sockid, PRIO_NORMAL);
}

void WorkManager::dispatchEventConnected(EventReceiver * er, int sockid, Prio prio) {
  eventqueues[prio]->push(Event(er, WORK_CONNECTED, sockid));
  event.post();
}

void WorkManager::dispatchEventDisconnected(EventReceiver * er, int sockid) {
  dispatchEventDisconnected(er, sockid, PRIO_NORMAL);
}

void WorkManager::dispatchEventDisconnected(EventReceiver * er, int sockid, Prio prio) {
  eventqueues[prio]->push(Event(er, WORK_DISCONNECTED, sockid));
  event.post();
}

void WorkManager::dispatchEventSSLSuccess(EventReceiver * er, int sockid, const std::string & cipher) {
  dispatchEventSSLSuccess(er, sockid, cipher, PRIO_NORMAL);
}

void WorkManager::dispatchEventSSLSuccess(EventReceiver * er, int sockid, const std::string & cipher, Prio prio) {
  eventqueues[prio]->push(Event(er, WORK_SSL_SUCCESS, sockid, cipher));
  event.post();
}

void WorkManager::dispatchEventSSLFail(EventReceiver * er, int sockid) {
  dispatchEventSSLFail(er, sockid, PRIO_NORMAL);
}

void WorkManager::dispatchEventSSLFail(EventReceiver * er, int sockid, Prio prio) {
  eventqueues[prio]->push(Event(er, WORK_SSL_FAIL, sockid));
  event.post();
}

void WorkManager::dispatchEventFail(EventReceiver * er, int sockid, const std::string & error) {
  dispatchEventFail(er, sockid, error, PRIO_NORMAL);
}

void WorkManager::dispatchEventFail(EventReceiver * er, int sockid, const std::string & error, Prio prio) {
  eventqueues[prio]->push(Event(er, WORK_FAIL, sockid, error));
  event.post();
}

void WorkManager::dispatchEventSendComplete(EventReceiver * er, int sockid) {
  dispatchEventSendComplete(er, sockid, PRIO_NORMAL);
}

void WorkManager::dispatchEventSendComplete(EventReceiver * er, int sockid, Prio prio) {
  eventqueues[prio]->push(Event(er, WORK_SEND_COMPLETE, sockid));
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
  lowprioqueue.push(Event(er, WORK_DELETE));
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

bool WorkManager::overload() {
  bool currentlyoverloaded = highprioqueue.size() + dataqueue.size() >= OVERLOADSIZE;
  if (currentlyoverloaded) {
    overloaded = true;
    lowpriooverloaded = true;
  }
  return overloaded;
}

bool WorkManager::lowPrioOverload() {
  bool currentlyoverloaded = highprioqueue.size() + dataqueue.size() + lowprioqueue.size() >= OVERLOADSIZE;
  if (currentlyoverloaded) {
    lowpriooverloaded = true;
  }
  return lowpriooverloaded;
}

void WorkManager::addReadyNotify(EventReceiver * er) {
  ScopeLock lock(readylock);
  readynotify.push_back(er);
}

void WorkManager::run() {
  while (true) {
    event.wait();
    if (signalevents.hasEvent()) {
      SignalData signal = signalevents.getClearFirst();
      signal.er->signal(signal.signal, signal.value);
    }
    else {
      Event event;
      if (highprioqueue.size()) {
        event = highprioqueue.pop();
      }
      else if (dataqueue.size()) {
        event = dataqueue.pop();
      }
      else {
        event = lowprioqueue.pop();
      }
      EventReceiver * er = event.getReceiver();
      int numdata = event.getNumericalData();
      switch (event.getType()) {
        case WORK_DATA:
          er->FDData(numdata);
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
          er->FDSSLSuccess(numdata, event.getStrData());
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
      if ((overloaded || lowpriooverloaded) &&
          dataqueue.size() + highprioqueue.size() + lowprioqueue.size() <= READYSIZE)
      {
        overloaded = false;
        lowpriooverloaded = false;
        ScopeLock lock(readylock);
        std::list<EventReceiver *>::iterator it;
        for (it = readynotify.begin(); it != readynotify.end(); it++) {
          (*it)->workerReady();
        }
      }
    }
  }
}
