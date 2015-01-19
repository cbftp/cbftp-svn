#pragma once

#include <string>

#define PENDINGTRANSFER_DOWNLOAD 5432
#define PENDINGTRANSFER_UPLOAD 5433
#define PENDINGTRANSFER_FXP 5434

class SiteLogic;
class FileList;

class PendingTransfer {
public:
  PendingTransfer(SiteLogic *, FileList *, std::string, SiteLogic *, FileList *, std::string);
  PendingTransfer(SiteLogic *, FileList *, std::string, std::string, std::string);
  PendingTransfer(std::string, std::string, SiteLogic *, FileList *, std::string);
  int type() const;
  SiteLogic * getSrc() const;
  SiteLogic * getDst() const;
  FileList * getSrcFileList() const;
  FileList * getDstFileList() const;
  std::string getSrcFileName() const;
  std::string getDstFileName() const;
  std::string getPath() const;
private:
  SiteLogic * slsrc;
  SiteLogic * sldst;
  FileList * flsrc;
  FileList * fldst;
  std::string srcfilename;
  std::string dstfilename;
  std::string path;
  int transfertype;
};
