#pragma once

class RequestCallback {
public:
  virtual ~RequestCallback() = default;
  virtual void requestReady(void* service, int requestid) = 0;
};
