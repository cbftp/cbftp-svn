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
  bool care;
public:
  SiteLogicRequest(int, int, int, bool);
  SiteLogicRequest(int, int, const std::string &, bool);
  SiteLogicRequest(int, int, const std::string &, const std::string &, bool);
  SiteLogicRequest(int, int, const std::string &, const std::string &, int, bool);
  int requestId() const;
  int requestType() const;
  std::string requestData() const;
  std::string requestData2() const;
  int requestData3() const;
  void setConnId(int);
  int connId() const;
  bool doesAnyoneCare() const;
};
