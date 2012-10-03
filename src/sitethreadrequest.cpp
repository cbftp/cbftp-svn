#include "sitethreadrequest.h"

SiteThreadRequest::SiteThreadRequest(int requestid, std::string path) {
  this->requestid = requestid;
  this->path = path;
}

int SiteThreadRequest::requestId() {
  return requestid;
}

std::string SiteThreadRequest::requestPath() {
  return path;
}
