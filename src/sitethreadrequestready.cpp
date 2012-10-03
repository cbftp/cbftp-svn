#include "sitethreadrequestready.h"

SiteThreadRequestReady::SiteThreadRequestReady(int requestid, FileList * filelist) {
  this->requestid = requestid;
  this->filelist = filelist;
}

int SiteThreadRequestReady::requestId() {
  return requestid;
}

FileList * SiteThreadRequestReady::requestFileList() {
  return filelist;
}
