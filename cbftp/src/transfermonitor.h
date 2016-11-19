#pragma once

#include <string>
#include <utility>

#include "core/eventreceiver.h"
#include "core/pointer.h"
#include "rawbuffercallback.h"
#include "path.h"

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
  TM_ERR_LOCK_UP,
  TM_ERR_DUPE
};

class SiteLogic;
class FileList;
class TransferManager;
class TransferStatus;
class LocalTransfer;
class LocalFileList;

class TransferMonitor : public EventReceiver, public RawBufferCallback {
  private:
    Status status;
    std::string sfile;
    std::string dfile;
    int src;
    int dst;
    int storeid;
    Pointer<SiteLogic> sls;
    Pointer<SiteLogic> sld;
    FileList * fls;
    FileList * fld;
    Pointer<LocalFileList> localfl;
    Path spath;
    Path dpath;
    bool clientactive;
    bool fxpdstactive;
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
    int ticker;
    TransferError error;
    void finish();
    void setTargetSizeSpeed(unsigned long long int, int);
    void reset();
    void transferFailed(const Pointer<TransferStatus> &, TransferError);
    void updateFXPSizeSpeed();
    void updateLocalTransferSizeSpeed();
    void checkForDeadFXPTransfers();
    void startClientTransfer();
  public:
    TransferMonitor(TransferManager *);
    ~TransferMonitor();
    void tick(int);
    void sourceComplete();
    void targetComplete();
    void sourceError(TransferError);
    void targetError(TransferError);
    void passiveReady(const std::string &, int);
    void activeReady();
    void activeStarted();
    void cipher(const std::string &);
    bool idle() const;
    Pointer<TransferStatus> getTransferStatus() const;
    void engageFXP(const std::string &, const Pointer<SiteLogic> &, FileList *, const std::string &, const Pointer<SiteLogic> &, FileList *);
    void engageDownload(const std::string &, const Pointer<SiteLogic> &, FileList *, const Pointer<LocalFileList> &);
    void engageUpload(const std::string &, const Pointer<LocalFileList> &, const Pointer<SiteLogic> &, FileList *);
    void engageList(const Pointer<SiteLogic> &, int, bool);
    Status getStatus() const;
    void newRawBufferLine(const std::pair<std::string, std::string> &);
};
