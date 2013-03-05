#pragma once

#include <string>

class SiteThreadRequest {
private:
  int requestid;
  int requesttype;
  std::string data;
public:
  SiteThreadRequest(int, int, std::string);
  int requestId();
  int requestType();
  std::string requestData();
};
