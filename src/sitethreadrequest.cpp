#include "sitethreadrequest.h"

SiteThreadRequest::SiteThreadRequest(int requestid, int requesttype, std::string data) {
  this->requestid = requestid;
  this->requesttype = requesttype;
  this->data = data;
}

int SiteThreadRequest::requestId() {
  return requestid;
}

int SiteThreadRequest::requestType() {
  return requesttype;
}

std::string SiteThreadRequest::requestData() {
  return data;
}
