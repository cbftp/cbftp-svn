#include "transferjob.h"

#include "filelist.h"
#include "file.h"

TransferJob::TransferJob(SiteLogic * sl, std::string srcfile, FileList * filelist, std::string path, std::string dstfile) :
      src(sl),
      srcfile(srcfile),
      dstfile(dstfile),
      dstpath(path),
      srclist(filelist) {
  type = srclist->getFile(srcfile)->isDirectory() ? TRANSFERJOB_DOWNLOAD : TRANSFERJOB_DOWNLOAD_FILE;
}

TransferJob::TransferJob(std::string path, std::string srcfile, SiteLogic * sl, std::string dstfile, FileList * filelist) :
      dst(sl),
      srcfile(srcfile),
      dstfile(dstfile),
      srcpath(path),
      dstlist(filelist) {
  type = TRANSFERJOB_UPLOAD;
}

TransferJob::TransferJob(SiteLogic * slsrc, std::string srcfile, FileList * srcfilelist, SiteLogic * sldst, std::string dstfile, FileList * dstfilelist) :
      src(slsrc),
      dst(sldst),
      srcfile(srcfile),
      dstfile(dstfile),
      srclist(srcfilelist),
      dstlist(dstfilelist) {
  type = srclist->getFile(srcfile)->isDirectory() ? TRANSFERJOB_FXP : TRANSFERJOB_FXP_FILE;
}

std::string TransferJob::getSrcFileName() const {
  return srcfile;
}

int TransferJob::getType() const {
  return type;
}
