#pragma once

class SiteLogicRequestReady {
private:
  int requestid;
  void * data;
public:
  SiteLogicRequestReady(int, void *);
  int requestId();
  void * requestData();
};
