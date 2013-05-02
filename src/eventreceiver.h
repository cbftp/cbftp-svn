#pragma once

class EventReceiver {
public:
  virtual ~EventReceiver();
  virtual void tick(int);
  virtual void FDConnected();
  virtual void FDData();
  virtual void FDData(char *, unsigned int);
  virtual void FDDisconnected();
};
