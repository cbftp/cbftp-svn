#pragma once

#include <string>

#include "eventreceiver.h"

class SiteLogic;
class FileList;
class TransferManager;

class TransferMonitor : public EventReceiver {
  private:
    int status;
    std::string file;
    int src;
    int dst;
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
    bool fxp;
    bool passiveready;
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
    bool idle();
    void engage(std::string, SiteLogic *, FileList *, SiteLogic *, FileList *);
    void engage(std::string, SiteLogic *, FileList *);
};
