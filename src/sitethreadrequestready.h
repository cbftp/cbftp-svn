#pragma once

#include <string>

class FileList;

class SiteThreadRequestReady {
private:
  int requestid;
  void * data;
public:
  SiteThreadRequestReady(int, void *);
  int requestId();
  void * requestData();
};
