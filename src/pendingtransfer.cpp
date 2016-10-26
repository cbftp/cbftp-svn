#include "pendingtransfer.h"

#include "localfilelist.h"

PendingTransfer::PendingTransfer(const Pointer<SiteLogic> & slsrc, FileList * flsrc, std::string srcfilename, const Pointer<SiteLogic> & sldst, FileList * fldst, std::string dstfilename) :
  slsrc(slsrc),
  sldst(sldst),
  flsrc(flsrc),
  fldst(fldst),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_FXP) {
}

PendingTransfer::PendingTransfer(const Pointer<SiteLogic> & sl, FileList * fl, std::string srcfilename, Pointer<LocalFileList> fllocal, std::string dstfilename) :
  slsrc(sl),
  flsrc(fl),
  fllocal(fllocal),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_DOWNLOAD) {
}

PendingTransfer::PendingTransfer(Pointer<LocalFileList> fllocal, std::string srcfilename, const Pointer<SiteLogic> & sl, FileList * fl, std::string dstfilename) :
  sldst(sl),
  fldst(fl),
  fllocal(fllocal),
  srcfilename(srcfilename),
  dstfilename(dstfilename),
  transfertype(PENDINGTRANSFER_UPLOAD) {
}

PendingTransfer::~PendingTransfer() {

}

int PendingTransfer::type() const {
  return transfertype;
}

const Pointer<SiteLogic> & PendingTransfer::getSrc() const {
  return slsrc;
}

const Pointer<SiteLogic> & PendingTransfer::getDst() const {
  return sldst;
}

FileList * PendingTransfer::getSrcFileList() const {
  return flsrc;
}

FileList * PendingTransfer::getDstFileList() const {
  return fldst;
}

Pointer<LocalFileList> & PendingTransfer::getLocalFileList() {
  return fllocal;
}

std::string PendingTransfer::getSrcFileName() const {
  return srcfilename;
}

std::string PendingTransfer::getDstFileName() const {
  return dstfilename;
}
