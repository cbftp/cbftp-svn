#pragma once

#include <string>

#define TRANSFERJOB_DOWNLOAD 2161
#define TRANSFERJOB_DOWNLOAD_FILE 2162
#define TRANSFERJOB_UPLOAD 2163
#define TRANSFERJOB_UPLOAD_FILE 2164
#define TRANSFERJOB_FXP 2165
#define TRANSFERJOB_FXP_FILE 2166

class SiteLogic;
class FileList;

class TransferJob {
public:
  TransferJob(SiteLogic *, std::string, FileList *, std::string, std::string);
  TransferJob(std::string, std::string, SiteLogic *, std::string, FileList *);
  TransferJob(SiteLogic *, std::string, FileList *, SiteLogic *, std::string, FileList *);
  std::string getSrcFileName() const;
  int getType() const;
private:
  int type;
  SiteLogic * src;
  SiteLogic * dst;
  std::string srcfile;
  std::string dstfile;
  std::string srcpath;
  std::string dstpath;
  FileList * srclist;
  FileList * dstlist;
  int slots;
};
