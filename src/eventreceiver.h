#pragma once

#include <pthread.h>
#include <string>

class EventReceiver {
private:
  pthread_mutex_t eventlock;
public:
  EventReceiver();
  virtual ~EventReceiver();
  virtual void tick(int);
  virtual void FDNew(int);
  virtual void FDConnected();
  virtual void FDData();
  virtual void FDData(char *, unsigned int);
  virtual void FDDisconnected();
  virtual void FDFail(std::string);
  virtual void FDSSLSuccess();
  virtual void FDSSLFail();
  virtual void lock();
  virtual void unlock();
};
