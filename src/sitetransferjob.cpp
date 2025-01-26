#include "sitetransferjob.h"

#include "filelist.h"
#include "transferjob.h"

SiteTransferJob::SiteTransferJob(const std::shared_ptr<TransferJob>& tj, bool source) : transferjob(tj), source(source) {

}

std::weak_ptr<TransferJob> SiteTransferJob::getTransferJob() {
  return transferjob;
}

std::shared_ptr<SiteLogic> SiteTransferJob::getOtherSiteLogic() const {
  return source ? transferjob.lock()->getDst() : transferjob.lock()->getSrc();
}

bool SiteTransferJob::otherWantsList() const {
  return transferjob.lock()->getSrc() != transferjob.lock()->getDst() && !transferjob.lock()->getListTargets(!source).empty();
}

bool SiteTransferJob::tryReserveListTarget(const std::shared_ptr<FileList>& fl, int connid) {
  return transferjob.lock()->tryReserveListTarget(fl, connid);
}

bool SiteTransferJob::isDone() const {
  std::shared_ptr<TransferJob> tj = transferjob.lock();
  return !tj || tj->isDone();
}

Path SiteTransferJob::getPath() const {
  return transferjob.lock()->getPath(source);
}

std::list<std::shared_ptr<FileList>> SiteTransferJob::getListTargets() const {
  return transferjob.lock()->getListTargets(source);
}

int SiteTransferJob::classType() const {
  return COMMANDOWNER_TRANSFERJOB;
}

std::string SiteTransferJob::getName() const {
  return transferjob.lock()->getName();
}

unsigned int SiteTransferJob::getId() const {
  return transferjob.lock()->getId();
}

void SiteTransferJob::fileListUpdated(SiteLogic * sl, const std::shared_ptr<FileList>& fl) {
  std::shared_ptr<TransferJob> tj = transferjob.lock();
  if (tj) {
    tj->fileListUpdated(source, fl);
  }
}

std::shared_ptr<FileList> SiteTransferJob::getFileListForFullPath(SiteLogic* sl, const Path& path) const {
  return transferjob.lock()->getFileListForFullPath(source, path);
}

bool SiteTransferJob::isRootFileList(const std::shared_ptr<FileList>& fl) const {
  std::shared_ptr<TransferJob> tj = transferjob.lock();
  if (!tj) {
    return false;
  }
  return tj->isRootFileList(source, fl);
}
