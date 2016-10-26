#pragma once

#include <string>

#include "core/pointer.h"

#define PENDINGTRANSFER_DOWNLOAD 5432
#define PENDINGTRANSFER_UPLOAD 5433
#define PENDINGTRANSFER_FXP 5434

class SiteLogic;
class FileList;
class LocalFileList;

class PendingTransfer {
public:
  PendingTransfer(const Pointer<SiteLogic> &, FileList *, std::string, const Pointer<SiteLogic> &, FileList *, std::string);
  PendingTransfer(const Pointer<SiteLogic> &, FileList *, std::string, Pointer<LocalFileList>, std::string);
  PendingTransfer(Pointer<LocalFileList>, std::string, const Pointer<SiteLogic> &, FileList *, std::string);
  ~PendingTransfer();
  int type() const;
  const Pointer<SiteLogic> & getSrc() const;
  const Pointer<SiteLogic> & getDst() const;
  FileList * getSrcFileList() const;
  FileList * getDstFileList() const;
  Pointer<LocalFileList> & getLocalFileList();
  std::string getSrcFileName() const;
  std::string getDstFileName() const;
private:
  Pointer<SiteLogic> slsrc;
  Pointer<SiteLogic> sldst;
  FileList * flsrc;
  FileList * fldst;
  Pointer<LocalFileList> fllocal;
  std::string srcfilename;
  std::string dstfilename;
  int transfertype;
};
