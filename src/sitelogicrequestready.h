#pragma once

class SiteLogicRequestReady {
private:
  int requestid;
  void * data;
  bool status;
public:
  SiteLogicRequestReady(int, void *, bool);
  int requestId() const;
  void * requestData() const;
  bool requestStatus() const;
};
