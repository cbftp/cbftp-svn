#pragma once

#include "sitelogic.h"
#include "siterace.h"
#include "ftpconn.h"
#include "filelist.h"
#include "transfermonitorbase.h"

class TransferMonitor : public TransferMonitorBase {
  private:
    int status;
    std::string file;
    int src;
    int dst;
    SiteLogic * sts;
    SiteLogic * std;
    FileList * fls;
    FileList * fld;
    bool passivedownload;
    bool sourcecomplete;
    bool targetcomplete;
  public:
    TransferMonitor();
    void tick(int);
    void sourceComplete();
    void targetComplete();
    void sourceError(int);
    void targetError(int);
    void passiveReady(std::string);
    bool idle();
    void engage(std::string, SiteLogic *, FileList *, SiteLogic *, FileList *);
};
