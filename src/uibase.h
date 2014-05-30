#pragma once

class UIBase {
public:
  virtual void backendPush() = 0;
  virtual void terminalSizeChanged() = 0;
  virtual ~UIBase() {
  }
};
