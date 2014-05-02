#pragma once

#include <string>

#include "eventreceiver.h"

#define TM_TYPE_FXP 960
#define TM_TYPE_LOCAL 961
#define TM_TYPE_LIST 962

#define TM_STATUS_IDLE 970
#define TM_STATUS_AWAITING_PASSIVE 971
#define TM_STATUS_AWAITING_ACTIVE 972
#define TM_STATUS_TRANSFERRING 973
#define TM_STATUS_ERROR_AWAITING_PEER 974

class SiteLogic;
class FileList;
class TransferManager;

class TransferMonitor : public EventReceiver {
  private:
    int status;
    std::string sfile;
    std::string dfile;
    int src;
    int dst;
    int storeid;
    SiteLogic * sls;
    SiteLogic * sld;
    FileList * fls;
    FileList * fld;
    std::string spath;
    std::string dpath;
    bool activedownload;
    bool sourcecomplete;
    bool targetcomplete;
    bool ssl;
    int type;
    int timestamp;
    int startstamp;
    TransferManager * tm;
    void finish();
  public:
    TransferMonitor(TransferManager *);
    void tick(int);
    void sourceComplete();
    void targetComplete();
    void localDownloadError(int);
    void sourceError(int);
    void targetError(int);
    void passiveReady(std::string);
    void activeReady();
    bool idle();
    // for FXP
    void engage(std::string, SiteLogic *, FileList *, std::string, SiteLogic *, FileList *);
    // for download
    void engage(std::string, SiteLogic *, FileList *);
    // for LIST
    void engage(SiteLogic *, int);
};
