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

int SiteLogicRequest::requestId() {
  return requestid;
}

int SiteLogicRequest::requestType() {
  return requesttype;
}

std::string SiteLogicRequest::requestData() {
  return data;
}

std::string SiteLogicRequest::requestData2() {
  return data2;
}

int SiteLogicRequest::requestData3() {
  return data3;
}
void SiteLogicRequest::setConnId(int id) {
  connid = id;
}

int SiteLogicRequest::connId() {
  return connid;
}
