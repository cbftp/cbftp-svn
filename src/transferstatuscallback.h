#pragma once

#include <memory>

class TransferStatus;

class TransferStatusCallback {
public:
  virtual ~TransferStatusCallback() {}
  virtual void transferFailed(const std::shared_ptr<TransferStatus> &, int) = 0;
  virtual void transferSuccessful(const std::shared_ptr<TransferStatus> &) = 0;
};
