#include "sitelogicrequestready.h"

SiteLogicRequestReady::SiteLogicRequestReady(int type, int requestid, void * data, bool status) :
  type(type),
  requestid(requestid),
  data(data),
  status(status) {
}

int SiteLogicRequestReady::getType() const {
  return type;
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
