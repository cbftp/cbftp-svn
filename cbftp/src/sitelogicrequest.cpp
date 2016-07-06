#include "sitelogicrequest.h"

SiteLogicRequest::SiteLogicRequest(int requestid, int requesttype, int data3, bool care) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data3(data3),
  care(care) {
}

SiteLogicRequest::SiteLogicRequest(int requestid, int requesttype, std::string data, bool care) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  care(care) {
}

SiteLogicRequest::SiteLogicRequest(int requestid, int requesttype, std::string data, std::string data2, int data3, bool care) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  data2(data2),
  data3(data3),
  care(care) {
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

bool SiteLogicRequest::doesAnyoneCare() const {
  return care;
}
