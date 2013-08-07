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
    SiteLogic * sls;
    SiteLogic * sld;
    FileList * fls;
    FileList * fld;
    bool activedownload;
    bool sourcecomplete;
    bool targetcomplete;
    bool ssl;
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
