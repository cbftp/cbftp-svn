#include "pendingtransfer.h"

PendingTransfer::PendingTransfer(SiteLogic * slsrc, FileList * flsrc, std::string srcfilename, SiteLogic * sldst, FileList * fldst, std::string dstfilename) :
  slsrc(slsrc),
  sldst(sldst),
  flsrc(flsrc),
  fldst(fldst),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_FXP) {
}

PendingTransfer::PendingTransfer(SiteLogic * sl, FileList * fl, std::string srcfilename, std::string path, std::string dstfilename) :
  slsrc(sl),
  flsrc(fl),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  path(path),
  transfertype(PENDINGTRANSFER_DOWNLOAD) {
}

PendingTransfer::PendingTransfer(std::string path, std::string srcfilename, SiteLogic * sl, FileList * fl, std::string dstfilename) :
  sldst(sl),
  fldst(fl),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  path(path),
  transfertype(PENDINGTRANSFER_UPLOAD) {
}

int PendingTransfer::type() const {
  return transfertype;
}

SiteLogic * PendingTransfer::getSrc() const {
  return slsrc;
}

SiteLogic * PendingTransfer::getDst() const {
  return sldst;
}

FileList * PendingTransfer::getSrcFileList() const {
  return flsrc;
}

FileList * PendingTransfer::getDstFileList() const {
  return fldst;
}

std::string PendingTransfer::getSrcFileName() const {
  return srcfilename;
}

std::string PendingTransfer::getDstFileName() const {
  return dstfilename;
}

std::string PendingTransfer::getPath() const {
  return path;
}
