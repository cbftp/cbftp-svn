#pragma once

#include <string>

class SiteThreadRequest {
private:
  int requestid;
  std::string path;
public:
  SiteThreadRequest(int, std::string);
  int requestId();
  std::string requestPath();
};
