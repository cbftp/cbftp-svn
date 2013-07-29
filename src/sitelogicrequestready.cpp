#include "sitelogicrequestready.h"

SiteLogicRequestReady::SiteLogicRequestReady(int requestid, void * data) {
  this->requestid = requestid;
  this->data = data;
}

int SiteLogicRequestReady::requestId() {
  return requestid;
}

void * SiteLogicRequestReady::requestData() {
  return data;
}
