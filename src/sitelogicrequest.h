#pragma once

#include <string>

class SiteLogicRequest {
private:
  int requestid;
  int connid;
  int requesttype;
  std::string data;
public:
  SiteLogicRequest(int, int, std::string);
  int requestId();
  int requestType();
  std::string requestData();
  void setConnId(int);
  int connId();
};
