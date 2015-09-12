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
  TM_STATUS_TRANSFERRING_SOURCE_COMPLETE,
  TM_STATUS_TRANSFERRING_TARGET_COMPLETE,
  TM_STATUS_SOURCE_ERROR_AWAITING_TARGET,
  TM_STATUS_TARGET_ERROR_AWAITING_SOURCE
};

enum TransferError {
  TM_ERR_PRET,
  TM_ERR_RETRSTOR,
  TM_ERR_RETRSTOR_COMPLETE,
  TM_ERR_OTHER,
  TM_ERR_LOCK_DOWN,
  TM_ERR_LOCK_UP
};

#define MAX_WAIT_ERROR 10000
#define MAX_WAIT_SOURCE_COMPLETE 60000

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
    bool ssl;
    TransferMonitorType type;
    int timestamp;
    int startstamp;
    int partialcompletestamp;
    TransferManager * tm;
    Pointer<TransferStatus> ts;
    int latesttouch;
    bool hiddenfiles;
    LocalTransfer * lt;
    int localtransferspeedticker;
    int checkdeadticker;
    void finish(bool);
    void setTargetSizeSpeed(unsigned int, int);
    void reset();
    void transferFailed(Pointer<TransferStatus>, TransferError);
    void updateFXPSizeSpeed();
    void updateLocalTransferSizeSpeed();
    void checkForDeadFXPTransfers();
  public:
    TransferMonitor(TransferManager *);
    ~TransferMonitor();
    void tick(int);
    void sourceComplete();
    void targetComplete();
    void sourceError(TransferError);
    void targetError(TransferError);
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
