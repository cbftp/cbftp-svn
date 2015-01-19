#pragma once

#include <string>

#include "eventreceiver.h"
#include "pointer.h"

#define TM_TYPE_FXP 960
#define TM_TYPE_LOCAL 961
#define TM_TYPE_LIST 962

#define TM_STATUS_IDLE 970
#define TM_STATUS_AWAITING_PASSIVE 971
#define TM_STATUS_AWAITING_ACTIVE 972
#define TM_STATUS_TRANSFERRING 973
#define TM_STATUS_ERROR_AWAITING_PEER 974

#define TICKINTERVAL 50

class SiteLogic;
class FileList;
class TransferManager;
class TransferStatus;
class LocalTransfer;

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
    Pointer<TransferStatus> ts;
    int latesttouch;
    bool hiddenfiles;
    LocalTransfer * lt;
    int localtransferspeedticker;
    void finish();
    void setTargetSizeSpeed(unsigned int, int);
    void reset();
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
    bool idle() const;
    Pointer<TransferStatus> getTransferStatus() const;
    void engageFXP(std::string, SiteLogic *, FileList *, std::string, SiteLogic *, FileList *);
    void engageDownload(std::string, SiteLogic *, FileList *, std::string);
    void engageUpload(std::string, std::string, SiteLogic *, FileList *);
    void engageList(SiteLogic *, int, bool);
    int getStatus() const;
};
