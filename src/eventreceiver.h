#pragma once

#include <pthread.h>

class EventReceiver {
private:
  pthread_mutex_t eventlock;
public:
  EventReceiver();
  virtual ~EventReceiver();
  virtual void tick(int);
  virtual void FDConnected();
  virtual void FDData();
  virtual void FDData(char *, unsigned int);
  virtual void FDDisconnected();
  virtual void FDSSLSuccess();
  virtual void FDSSLFail();
  void lock();
  void unlock();
};
