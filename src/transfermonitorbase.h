#pragma once

#include "eventreceiver.h"

class TransferMonitorBase : public EventReceiver {
public:
  virtual void tick(int) = 0;
  virtual void sourceComplete() = 0;
  virtual void targetComplete() = 0;
  virtual void sourceError(int) = 0;
  virtual void targetError(int) = 0;
  virtual void passiveReady(std::string) = 0;
};
