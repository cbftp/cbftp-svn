#pragma once

#include <string>

class SiteLogicRequest {
private:
  int requestid;
  int connid;
  int requesttype;
  std::string data;
  std::string data2;
  int data3;
public:
  SiteLogicRequest(int, int, std::string);
  SiteLogicRequest(int, int, std::string, std::string, int);
  int requestId();
  int requestType();
  std::string requestData();
  std::string requestData2();
  int requestData3();
  void setConnId(int);
  int connId();
};
