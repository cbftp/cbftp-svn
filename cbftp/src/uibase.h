#pragma once

class UIBase {
public:
  virtual void backendPush() = 0;
  virtual bool legendEnabled() const {
    return false;
  }
  virtual void showLegend(bool) {
  }
  virtual ~UIBase() {
  }
};
