#pragma once

#include <string>
#include <list>
#include <map>

#include "core/pointer.h"
#include "types.h"

class LocalTransfer;
class LocalDownload;
class LocalUpload;
class TransferMonitor;
class FTPConn;
class LocalFileList;

#define MAXREAD 524288

class LocalStorage {
public:
  LocalStorage();
  ~LocalStorage();
  LocalTransfer * passiveDownload(TransferMonitor *, std::string, std::string, bool, FTPConn *);
  LocalTransfer * passiveDownload(TransferMonitor *, std::string, std::string, std::string, bool, FTPConn *);
  LocalTransfer * passiveDownload(TransferMonitor *, std::string, bool, FTPConn *);
  LocalTransfer * passiveUpload(TransferMonitor *, std::string, std::string, std::string, bool, FTPConn *);
  binary_data getTempFileContent(const std::string &) const;
  binary_data getFileContent(const std::string &) const;
  const binary_data & getStoreContent(int) const;
  void purgeStoreContent(int);
  void deleteFile(std::string);
  std::string getTempPath() const;
  void setTempPath(std::string);
  void storeContent(int, const binary_data &);
  std::string getDownloadPath() const;
  void setDownloadPath(std::string);
  static Pointer<LocalFileList> getLocalFileList(std::string);
  unsigned long long int getFileSize(const std::string &);
  static bool directoryExistsReadable(std::string);
  static bool directoryExistsWritable(std::string);
  static bool createDirectory(std::string);
  static bool createDirectory(std::string, bool);
  static bool createDirectoryRecursive(std::string);
private:
  static bool directoryExistsAccessible(std::string, bool);
  std::map<int, binary_data> content;
  static std::string getHostFromPASVString(std::string);
  static int getPortFromPASVString(std::string);
  LocalDownload * getAvailableLocalDownload();
  LocalUpload * getAvailableLocalUpload();
  std::list<LocalDownload *> localdownloads;
  std::list<LocalUpload *> localuploads;
  std::string temppath;
  std::string downloadpath;
  int storeidcounter;
};
