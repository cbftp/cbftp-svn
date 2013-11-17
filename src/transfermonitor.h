#pragma once

#include <string>

#include "eventreceiver.h"

class SiteLogic;
class FileList;

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
    bool activedownload;
    bool sourcecomplete;
    bool targetcomplete;
    bool ssl;
    bool passiveready;
    int timestamp;
    int startstamp;
    void finish();
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
