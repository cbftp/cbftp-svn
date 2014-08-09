#include "sitelogicrequestready.h"

SiteLogicRequestReady::SiteLogicRequestReady(int requestid, void * data, bool status) {
  this->requestid = requestid;
  this->data = data;
  this->status = status;
}

int SiteLogicRequestReady::requestId() const {
  return requestid;
}

void * SiteLogicRequestReady::requestData() const {
  return data;
}

bool SiteLogicRequestReady::requestStatus() const {
  return status;
}
