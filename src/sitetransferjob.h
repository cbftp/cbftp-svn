#pragma once

#include <memory>
#include <string>

#include "commandowner.h"

class FileList;
class SiteLogic;
class TransferJob;

class SiteTransferJob : public CommandOwner {
public:
  SiteTransferJob(TransferJob* tj, bool source);
  TransferJob* getTransferJob();
  std::shared_ptr<SiteLogic> getOtherSiteLogic() const;
  bool wantsList();
  bool otherWantsList();
  Path getPath() const;
  std::shared_ptr<FileList> getListTarget();
  int classType() const override;
  std::string getName() const override;
  unsigned int getId() const override;
  void fileListUpdated(SiteLogic* sl, const std::shared_ptr<FileList>& fl) override;
  std::shared_ptr<FileList> getFileListForFullPath(SiteLogic* sl, const Path& path) const override;
private:
  TransferJob* transferjob;
  bool source;
};
