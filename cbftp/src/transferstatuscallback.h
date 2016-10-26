#pragma once

#include "core/pointer.h"

class TransferStatus;

class TransferStatusCallback {
public:
  virtual ~TransferStatusCallback() {}
  virtual void transferFailed(const Pointer<TransferStatus> &, int) = 0;
  virtual void transferSuccessful(const Pointer<TransferStatus> &) = 0;
};
