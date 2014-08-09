#include "sitelogicrequest.h"

SiteLogicRequest::SiteLogicRequest(int requestid, int requesttype, std::string data) {
  this->requestid = requestid;
  this->requesttype = requesttype;
  this->data = data;
  this->connid = -1;
}

SiteLogicRequest::SiteLogicRequest(int requestid, int requesttype, std::string data, std::string data2, int data3) {
  this->requestid = requestid;
  this->requesttype = requesttype;
  this->data = data;
  this->data2 = data2;
  this->data3 = data3;
  this->connid = -1;
}

int SiteLogicRequest::requestId() const {
  return requestid;
}

int SiteLogicRequest::requestType() const {
  return requesttype;
}

std::string SiteLogicRequest::requestData() const {
  return data;
}

std::string SiteLogicRequest::requestData2() const {
  return data2;
}

int SiteLogicRequest::requestData3() const {
  return data3;
}
void SiteLogicRequest::setConnId(int id) {
  connid = id;
}

int SiteLogicRequest::connId() const {
  return connid;
}
