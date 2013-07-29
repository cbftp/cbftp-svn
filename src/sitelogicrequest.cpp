#include "sitelogicrequest.h"

SiteLogicRequest::SiteLogicRequest(int requestid, int requesttype, std::string data) {
  this->requestid = requestid;
  this->requesttype = requesttype;
  this->data = data;
  this->connid = -1;
}

int SiteLogicRequest::requestId() {
  return requestid;
}

int SiteLogicRequest::requestType() {
  return requesttype;
}

std::string SiteLogicRequest::requestData() {
  return data;
}

void SiteLogicRequest::setConnId(int id) {
  connid = id;
}

int SiteLogicRequest::connId() {
  return connid;
}
