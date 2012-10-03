#pragma once

#include <string>

class FileList;

class SiteThreadRequestReady {
private:
  int requestid;
  FileList * filelist;
public:
  SiteThreadRequestReady(int, FileList *);
  int requestId();
  FileList * requestFileList();
};
