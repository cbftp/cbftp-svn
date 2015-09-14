#pragma once

#include <string>
#include <list>
#include <map>

#include "pointer.h"
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
  void readConfiguration();
  void writeState();
  LocalTransfer * passiveDownload(TransferMonitor *, std::string, std::string, bool, FTPConn *);
  LocalTransfer * passiveDownload(TransferMonitor *, std::string, std::string, std::string, bool, FTPConn *);
  LocalTransfer * passiveDownload(TransferMonitor *, std::string, bool, FTPConn *);
  LocalTransfer * passiveUpload(TransferMonitor *, std::string, std::string, std::string, bool, FTPConn *);
  binary_data getFileContent(std::string) const;
  const binary_data & getStoreContent(int) const;
  void purgeStoreContent(int);
  void deleteFile(std::string);
  std::string getTempPath() const;
  void setTempPath(std::string);
  void storeContent(int, const binary_data &);
  std::string getDownloadPath() const;
  void setDownloadPath(std::string);
  Pointer<LocalFileList> getLocalFileList(std::string);
  bool directoryExistsReadable(std::string);
  bool directoryExistsWritable(std::string);
  bool createDirectory(std::string);
  bool createDirectory(std::string, bool);
  bool createDirectoryRecursive(std::string);
private:
  bool directoryExistsAccessible(std::string, bool);
  std::map<int, binary_data> content;
  std::string getHostFromPASVString(std::string) const;
  int getPortFromPASVString(std::string) const;
  LocalDownload * getAvailableLocalDownload();
  LocalUpload * getAvailableLocalUpload();
  std::list<LocalDownload *> localdownloads;
  std::list<LocalUpload *> localuploads;
  std::string temppath;
  std::string downloadpath;
  int storeidcounter;
};
