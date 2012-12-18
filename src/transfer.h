#pragma once

#include <semaphore.h>

#include "sitethread.h"
#include "siterace.h"
#include "ftpthread.h"
#include "filelist.h"

class Transfer {
  private:
    sem_t donesem;
    int status;
    std::string file;
    FTPThread * src;
    FTPThread * dst;
    SiteThread * sts;
    SiteThread * std;
    FileList * fls;
    FileList * fld;
  public:
    Transfer(std::string, SiteThread *, FileList *, SiteThread *, FileList *);
    void run();
    void cancelTransfer();
};
