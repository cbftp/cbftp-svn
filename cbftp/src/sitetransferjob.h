#pragma once

#include <memory>
#include <string>

#include "commandowner.h"

class FileList;
class SiteLogic;
class TransferJob;

class SiteTransferJob : public CommandOwner {
public:
  SiteTransferJob(TransferJob * tj, bool source);
  TransferJob * getTransferJob();
  std::shared_ptr<SiteLogic> getOtherSiteLogic() const;
  bool wantsList();
  bool otherWantsList();
  Path getPath() const;
  FileList * getListTarget();
  int classType() const;
  std::string getName() const;
  unsigned int getId() const;
  void fileListUpdated(SiteLogic *, FileList *);
  FileList * getFileListForFullPath(SiteLogic *, const Path &) const;
private:
  TransferJob * transferjob;
  bool source;
};
