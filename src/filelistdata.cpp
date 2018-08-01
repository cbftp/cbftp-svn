#include "filelistdata.h"

FileListData::FileListData(FileList * filelist, RawBuffer * cwdrawbuffer)
  : filelist(filelist), cwdrawbuffer(*cwdrawbuffer)
{

}

FileList * FileListData::getFileList() const {
  return filelist;
}

const RawBuffer & FileListData::getCwdRawBuffer() const {
  return cwdrawbuffer;
}
