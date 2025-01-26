#pragma once

#include <list>
#include <memory>
#include <string>

#include "commandowner.h"

class FileList;
class SiteLogic;
class TransferJob;

class SiteTransferJob : public CommandOwner {
public:
  SiteTransferJob(const std::shared_ptr<TransferJob>& tj, bool source);
  std::weak_ptr<TransferJob> getTransferJob();
  std::shared_ptr<SiteLogic> getOtherSiteLogic() const;
  bool otherWantsList() const;
  bool tryReserveListTarget(const std::shared_ptr<FileList>& fl, int connid);
  bool isDone() const;
  Path getPath() const;
  std::list<std::shared_ptr<FileList>> getListTargets() const;
  int classType() const override;
  std::string getName() const override;
  unsigned int getId() const override;
  void fileListUpdated(SiteLogic* sl, const std::shared_ptr<FileList>& fl) override;
  std::shared_ptr<FileList> getFileListForFullPath(SiteLogic* sl, const Path& path) const override;
  bool isRootFileList(const std::shared_ptr<FileList>& fl) const;
private:
  std::weak_ptr<TransferJob> transferjob;
  bool source;
};
