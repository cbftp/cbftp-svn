#include "transfermanager.h"

#include "scoreboardelement.h"
#include "transfermonitor.h"
#include "uibase.h"
#include "globalcontext.h"
#include "transferstatus.h"
#include "localfilelist.h"
#include "transferstatuscallback.h"
#include "engine.h"

TransferManager::TransferManager() : totalfinishedtransfers(0) {
}

TransferManager::~TransferManager() {

}

void TransferManager::getFileList(
  const std::shared_ptr<SiteLogic> & sl, int connid, bool hiddenfiles, FileList * fl, CommandOwner * co)
{
  std::shared_ptr<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageList(sl, connid, hiddenfiles, fl, co);
}

std::shared_ptr<TransferStatus> TransferManager::suggestTransfer(
  const std::string & name, const std::shared_ptr<SiteLogic> & src, FileList * fls,
  const std::shared_ptr<SiteLogic> & dst, FileList * fld, CommandOwner * srcco, CommandOwner * dstco)
{
  return suggestTransfer(name, src, fls, name, dst, fld, srcco, dstco);
}

std::shared_ptr<TransferStatus> TransferManager::suggestTransfer(
  const std::string & srcname, const std::shared_ptr<SiteLogic> & src, FileList * fls,
  const std::string & dstname, const std::shared_ptr<SiteLogic> & dst, FileList * fld,
  CommandOwner * srcco, CommandOwner * dstco)
{
  std::shared_ptr<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageFXP(srcname, src, fls, dstname, dst, fld, srcco, dstco);
  return target->getTransferStatus();
}

std::shared_ptr<TransferStatus> TransferManager::suggestDownload(
  const std::string & name, const std::shared_ptr<SiteLogic> & sl, FileList * filelist,
  const std::shared_ptr<LocalFileList> & path, CommandOwner * co)
{
  std::shared_ptr<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageDownload(name, sl, filelist, path, co);
  return target->getTransferStatus();
}

std::shared_ptr<TransferStatus> TransferManager::suggestUpload(
  const std::string & name, const std::shared_ptr<LocalFileList> & path,
  const std::shared_ptr<SiteLogic> & sl, FileList * filelist, CommandOwner * co)
{
  std::shared_ptr<TransferMonitor> target = getAvailableTransferMonitor();
  target->engageUpload(name, path, sl, filelist, co);
  return target->getTransferStatus();
}

std::shared_ptr<TransferMonitor> TransferManager::getAvailableTransferMonitor() {
  std::shared_ptr<TransferMonitor> target;
  std::list<std::shared_ptr<TransferMonitor> >::iterator it;
  for (it = transfermonitors.begin(); it != transfermonitors.end(); it++) {
    if ((*it)->idle()) {
      target = *it;
      break;
    }
  }
  if (!target) {
    target = std::make_shared<TransferMonitor>(this);
    transfermonitors.push_back(target);
  }
  return target;
}

void TransferManager::transferSuccessful(const std::shared_ptr<TransferStatus> & ts) {
  if (!!ts) {
    if (ts->isAwaited()) {
      global->getUIBase()->backendPush();
    }
    TransferStatusCallback * callback = ts->getCallback();
    if (callback != NULL) {
      callback->transferSuccessful(ts);
    }
    moveTransferStatusToFinished(ts);
  }
}

void TransferManager::transferFailed(const std::shared_ptr<TransferStatus> & ts, int err) {
  if (!!ts) {
    if (ts->isAwaited()) {
      global->getUIBase()->backendPush();
    }
    TransferStatusCallback * callback = ts->getCallback();
    if (callback != NULL) {
      callback->transferFailed(ts, err);
    }
    global->getEngine()->transferFailed(ts, err);
    moveTransferStatusToFinished(ts);
  }
}

std::list<std::shared_ptr<TransferStatus> >::const_iterator TransferManager::ongoingTransfersBegin() const {
  return ongoingtransfers.begin();
}

std::list<std::shared_ptr<TransferStatus> >::const_iterator TransferManager::ongoingTransfersEnd() const {
  return ongoingtransfers.end();
}

std::list<std::shared_ptr<TransferStatus> >::const_iterator TransferManager::finishedTransfersBegin() const {
  return finishedtransfers.begin();
}

std::list<std::shared_ptr<TransferStatus> >::const_iterator TransferManager::finishedTransfersEnd() const {
  return finishedtransfers.end();
}

unsigned int TransferManager::ongoingTransfersSize() const {
  return ongoingtransfers.size();
}

unsigned int TransferManager::finishedTransfersSize() const {
  return finishedtransfers.size();
}

void TransferManager::addNewTransferStatus(const std::shared_ptr<TransferStatus> & ts) {
  ongoingtransfers.push_front(ts);
}

void TransferManager::moveTransferStatusToFinished(const std::shared_ptr<TransferStatus> & movets) {
  for (std::list<std::shared_ptr<TransferStatus> >::iterator it = ongoingtransfers.begin(); it != ongoingtransfers.end(); it++) {
    if (*it == movets) {
      ongoingtransfers.erase(it);
      break;
    }
  }
  if (finishedtransfers.size() > MAX_TRANSFER_HISTORY) {
    finishedtransfers.pop_back();
  }
  finishedtransfers.push_front(movets);
  ++totalfinishedtransfers;
}

unsigned int TransferManager::totalFinishedTransfers() const {
  return totalfinishedtransfers;
}
