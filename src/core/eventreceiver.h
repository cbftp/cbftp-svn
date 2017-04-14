#pragma once

#include <string>

class EventReceiver {
public:
  virtual ~EventReceiver();
  virtual void tick(int);
  virtual void signal(int, int);
  virtual void FDNew(int);
  virtual void FDConnecting(int, const std::string &);
  virtual void FDConnected(int);
  virtual void FDData(int);
  virtual void FDData(int, char *, unsigned int);
  virtual void FDDisconnected(int);
  virtual void FDFail(int, const std::string &);
  virtual void FDSSLSuccess(int, const std::string &);
  virtual void FDSSLFail(int);
  virtual void FDSendComplete(int);
  virtual void asyncTaskComplete(int, void *);
  virtual void asyncTaskComplete(int, int);
  virtual void workerReady();
};
