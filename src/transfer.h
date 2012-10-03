#pragma once

#include <semaphore.h>

#include "sitethread.h"
#include "siterace.h"
#include "ftpthread.h"

class Transfer {
  private:
    sem_t donesem;
    int status;
    std::string file;
    FTPThread * src;
    FTPThread * dst;
    SiteThread * sts;
    SiteThread * std;
    SiteRace * srs;
    SiteRace * srd;
  public:
    Transfer(std::string, SiteThread *, SiteRace *, SiteThread *, SiteRace *);
    void run();
    void cancelTransfer();
};
