#pragma once

class UIBase {
public:
  virtual void backendPush() = 0;
  virtual ~UIBase() {
  }
};
