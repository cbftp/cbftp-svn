#pragma once

#include <string>
#include <list>

class EventReceiver {
public:
  virtual ~EventReceiver();
  virtual void tick(int);
  virtual void FDConnected();
  virtual void FDData(char *, unsigned int);
  virtual void FDDisconnected();
};
