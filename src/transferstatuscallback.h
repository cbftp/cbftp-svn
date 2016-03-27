#pragma once

#include "core/pointer.h"

class TransferStatus;

class TransferStatusCallback {
public:
  virtual ~TransferStatusCallback() {}
  virtual void transferFailed(Pointer<TransferStatus> &, int) = 0;
  virtual void transferSuccessful(Pointer<TransferStatus> &) = 0;
};
