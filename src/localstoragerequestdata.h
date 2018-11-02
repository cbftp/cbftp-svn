#include <memory>

#include "path.h"

class LocalFileList;
class LocalPathInfo;

enum class LocalStorageRequestType {
  GET_FILE_LIST,
  GET_PATH_INFO,
  DELETE
};

struct LocalStorageRequestData {
  LocalStorageRequestData();
  int requestid;
  bool care;
  LocalStorageRequestType type;
};

struct FileListTaskData : public LocalStorageRequestData {
  FileListTaskData();
  Path path;
  std::shared_ptr<LocalFileList> filelist;
};

struct PathInfoTaskData : public LocalStorageRequestData {
  PathInfoTaskData();
  std::list<Path> paths;
  std::shared_ptr<LocalPathInfo> pathinfo;
};

struct DeleteFileTaskData : public LocalStorageRequestData {
  DeleteFileTaskData();
  Path file;
  bool success;
};
