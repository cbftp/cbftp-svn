#include "sitethreadrequestready.h"

SiteThreadRequestReady::SiteThreadRequestReady(int requestid, void * data) {
  this->requestid = requestid;
  this->data = data;
}

int SiteThreadRequestReady::requestId() {
  return requestid;
}

void * SiteThreadRequestReady::requestData() {
  return data;
}
