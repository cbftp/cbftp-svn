#pragma once

#include <string>

#include "eventreceiver.h"
#include "pointer.h"

enum TransferMonitorType {
 TM_TYPE_FXP,
 TM_TYPE_DOWNLOAD,
 TM_TYPE_UPLOAD,
 TM_TYPE_LIST
};

enum Status {
  TM_STATUS_IDLE,
  TM_STATUS_AWAITING_PASSIVE,
  TM_STATUS_AWAITING_ACTIVE,
  TM_STATUS_TRANSFERRING,
  TM_STATUS_ERROR_AWAITING_PEER
};

#define TICKINTERVAL 50

class SiteLogic;
class FileList;
class TransferManager;
class TransferStatus;
class LocalTransfer;
class LocalFileList;

class TransferMonitor : public EventReceiver {
  private:
    Status status;
    std::string sfile;
    std::string dfile;
    int src;
    int dst;
    int storeid;
    SiteLogic * sls;
    SiteLogic * sld;
    FileList * fls;
    FileList * fld;
    Pointer<LocalFileList> localfl;
    std::string spath;
    std::string dpath;
    bool activedownload;
    bool sourcecomplete;
    bool targetcomplete;
    bool ssl;
    TransferMonitorType type;
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
    void transferFailed(Pointer<TransferStatus>, int);
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
    void engageDownload(std::string, SiteLogic *, FileList *, Pointer<LocalFileList>);
    void engageUpload(std::string, Pointer<LocalFileList>, SiteLogic *, FileList *);
    void engageList(SiteLogic *, int, bool);
    Status getStatus() const;
};
