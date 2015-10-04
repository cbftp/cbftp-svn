#pragma once

#include <string>

#include "pointer.h"

#define PENDINGTRANSFER_DOWNLOAD 5432
#define PENDINGTRANSFER_UPLOAD 5433
#define PENDINGTRANSFER_FXP 5434

class SiteLogic;
class FileList;
class LocalFileList;

class PendingTransfer {
public:
  PendingTransfer(SiteLogic *, FileList *, std::string, SiteLogic *, FileList *, std::string);
  PendingTransfer(SiteLogic *, FileList *, std::string, Pointer<LocalFileList>, std::string);
  PendingTransfer(Pointer<LocalFileList>, std::string, SiteLogic *, FileList *, std::string);
  ~PendingTransfer();
  int type() const;
  SiteLogic * getSrc() const;
  SiteLogic * getDst() const;
  FileList * getSrcFileList() const;
  FileList * getDstFileList() const;
  Pointer<LocalFileList> & getLocalFileList();
  std::string getSrcFileName() const;
  std::string getDstFileName() const;
private:
  SiteLogic * slsrc;
  SiteLogic * sldst;
  FileList * flsrc;
  FileList * fldst;
  Pointer<LocalFileList> fllocal;
  std::string srcfilename;
  std::string dstfilename;
  int transfertype;
};
