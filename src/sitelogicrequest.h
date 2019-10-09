#pragma once

#include <string>

class RequestCallback;

class SiteLogicRequest {
private:
  int requestid;
  int connid;
  int requesttype;
  std::string data;
  std::string data2;
  int data3;
  RequestCallback* cb;
public:
  SiteLogicRequest(RequestCallback* cb, int, int, int);
  SiteLogicRequest(RequestCallback* cb, int, int, const std::string &);
  SiteLogicRequest(RequestCallback* cb, int, int, const std::string &, int);
  SiteLogicRequest(RequestCallback* cb, int, int, const std::string &, const std::string &);
  SiteLogicRequest(RequestCallback* cb, int, int, const std::string &, const std::string &, int);
  int requestId() const;
  int requestType() const;
  std::string requestData() const;
  std::string requestData2() const;
  int requestData3() const;
  void setConnId(int);
  int connId() const;
  RequestCallback* callback() const;
};
