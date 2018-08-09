#pragma once

#include <memory>
#include <string>

#define PENDINGTRANSFER_DOWNLOAD 5432
#define PENDINGTRANSFER_UPLOAD 5433
#define PENDINGTRANSFER_FXP 5434

class SiteLogic;
class FileList;
class LocalFileList;

class PendingTransfer {
public:
  PendingTransfer(const std::shared_ptr<SiteLogic> &, FileList *, std::string, const std::shared_ptr<SiteLogic> &, FileList *, std::string);
  PendingTransfer(const std::shared_ptr<SiteLogic> &, FileList *, std::string, std::shared_ptr<LocalFileList>, std::string);
  PendingTransfer(std::shared_ptr<LocalFileList>, std::string, const std::shared_ptr<SiteLogic> &, FileList *, std::string);
  ~PendingTransfer();
  int type() const;
  const std::shared_ptr<SiteLogic> & getSrc() const;
  const std::shared_ptr<SiteLogic> & getDst() const;
  FileList * getSrcFileList() const;
  FileList * getDstFileList() const;
  std::shared_ptr<LocalFileList> & getLocalFileList();
  std::string getSrcFileName() const;
  std::string getDstFileName() const;
private:
  std::shared_ptr<SiteLogic> slsrc;
  std::shared_ptr<SiteLogic> sldst;
  FileList * flsrc;
  FileList * fldst;
  std::shared_ptr<LocalFileList> fllocal;
  std::string srcfilename;
  std::string dstfilename;
  int transfertype;
};
