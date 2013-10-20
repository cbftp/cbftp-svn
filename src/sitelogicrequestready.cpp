#include "sitelogicrequestready.h"

SiteLogicRequestReady::SiteLogicRequestReady(int requestid, void * data, bool status) {
  this->requestid = requestid;
  this->data = data;
  this->status = status;
}

int SiteLogicRequestReady::requestId() {
  return requestid;
}

void * SiteLogicRequestReady::requestData() {
  return data;
}

bool SiteLogicRequestReady::requestStatus() {
  return status;
}
