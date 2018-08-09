#include "sitetransferjob.h"

#include "transferjob.h"

SiteTransferJob::SiteTransferJob(TransferJob * tj, bool source) : transferjob(tj), source(source) {

}

TransferJob * SiteTransferJob::getTransferJob() {
  return transferjob;
}

std::shared_ptr<SiteLogic> SiteTransferJob::getOtherSiteLogic() const {
  return source ? transferjob->getDst() : transferjob->getSrc();
}

bool SiteTransferJob::wantsList() {
  return transferjob->wantsList(source);
}

bool SiteTransferJob::otherWantsList() {
  return transferjob->getSrc() != transferjob->getDst() && transferjob->wantsList(!source);
}

Path SiteTransferJob::getPath() const {
  return transferjob->getPath(source);
}

FileList * SiteTransferJob::getListTarget() {
  return transferjob->getListTarget(source);
}
int SiteTransferJob::classType() const {
  return COMMANDOWNER_TRANSFERJOB;
}

std::string SiteTransferJob::getName() const {
  return transferjob->getName();
}

unsigned int SiteTransferJob::getId() const {
  return transferjob->getId();
}
void SiteTransferJob::fileListUpdated(SiteLogic * sl, FileList * fl) {
  return transferjob->fileListUpdated(source, fl);
}

FileList * SiteTransferJob::getFileListForFullPath(SiteLogic * sl, const Path & path) const {
  return transferjob->getFileListForFullPath(source, path);
}
