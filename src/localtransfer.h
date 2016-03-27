#pragma once

#include "core/eventreceiver.h"

#define CHUNK 524288

class LocalTransfer : public EventReceiver {
public:
  virtual unsigned long long int size() const = 0;
};
