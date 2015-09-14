#pragma once

#include <string>

class EventReceiver {
public:
  virtual ~EventReceiver();
  virtual void tick(int);
  virtual void signal(int);
  virtual void FDNew(int);
  virtual void FDConnected();
  virtual void FDData();
  virtual void FDData(char *, unsigned int);
  virtual void FDDisconnected();
  virtual void FDFail(std::string);
  virtual void FDSSLSuccess();
  virtual void FDSSLFail();
  virtual void FDSendComplete();
};
