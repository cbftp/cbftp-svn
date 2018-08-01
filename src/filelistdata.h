#pragma once

#include "rawbuffer.h"

class FileList;

class FileListData {
public:
  FileListData(FileList * filelist, RawBuffer * cwdrawbuffer);
  FileList * getFileList() const;
  const RawBuffer & getCwdRawBuffer() const;
private:
  FileList * filelist;
  RawBuffer cwdrawbuffer;
};
