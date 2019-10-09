#include "sitelogicrequest.h"

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, int data3) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data3(data3),
  cb(cb)
{
}

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string & data) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  cb(cb)
{
}

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string & data, int data3) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  data3(data3),
  cb(cb)
{
}

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string & data, const std::string & data2) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  data2(data2),
  cb(cb)
{
}

SiteLogicRequest::SiteLogicRequest(RequestCallback* cb, int requestid, int requesttype, const std::string & data, const std::string & data2, int data3) :
  requestid(requestid),
  connid(-1),
  requesttype(requesttype),
  data(data),
  data2(data2),
  data3(data3),
  cb(cb)
{
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

RequestCallback* SiteLogicRequest::callback() const {
  return cb;
}
