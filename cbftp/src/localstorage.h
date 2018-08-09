#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>

#include "core/types.h"
#include "path.h"
#include "localpathinfo.h"
#include "localfile.h"

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
  LocalTransfer * passiveModeDownload(TransferMonitor *, const std::string &, const std::string &, int, bool, FTPConn *);
  LocalTransfer * passiveModeDownload(TransferMonitor *, const Path &, const std::string &, const std::string &, int, bool, FTPConn *);
  LocalTransfer * passiveModeDownload(TransferMonitor *, const std::string &, int, bool, FTPConn *);
  LocalTransfer * passiveModeUpload(TransferMonitor *, const Path &, const std::string &, const std::string &, int, bool, FTPConn *);
  LocalTransfer * activeModeDownload(TransferMonitor *, const Path &, const std::string &, bool, FTPConn *);
  LocalTransfer * activeModeDownload(TransferMonitor *, bool, FTPConn *);
  LocalTransfer * activeModeUpload(TransferMonitor *, const Path &, const std::string &, bool, FTPConn *);
  BinaryData getTempFileContent(const std::string &) const;
  BinaryData getFileContent(const Path &) const;
  const BinaryData & getStoreContent(int) const;
  void purgeStoreContent(int);
  bool deleteFile(const Path & filename);
  static bool deleteFileAbsolute(const Path & filename);
  static bool deleteRecursive(const Path & path);
  static LocalPathInfo getPathInfo(const Path & path);
  static LocalFile getLocalFile(const Path & path);
  const Path & getTempPath() const;
  void setTempPath(const std::string &);
  void storeContent(int, const BinaryData &);
  const Path & getDownloadPath() const;
  void setDownloadPath(const Path &);
  static std::shared_ptr<LocalFileList> getLocalFileList(const Path &);
  unsigned long long int getFileSize(const Path &);
  bool getUseActiveModeAddress() const;
  const std::string & getActiveModeAddress() const;
  int getActivePortFirst() const;
  int getActivePortLast() const;
  void setUseActiveModeAddress(bool);
  void setActiveModeAddress(const std::string &);
  void setActivePortFirst(int);
  void setActivePortLast(int);
  int getNextActivePort();
  std::string getAddress(LocalTransfer *) const;
private:
  static LocalPathInfo getPathInfo(const Path & path, int currentdepth);
  std::map<int, BinaryData> content;
  LocalDownload * getAvailableLocalDownload();
  LocalUpload * getAvailableLocalUpload();
  std::list<LocalDownload *> localdownloads;
  std::list<LocalUpload *> localuploads;
  Path temppath;
  Path downloadpath;
  int storeidcounter;
  bool useactivemodeaddress;
  std::string activemodeaddress;
  int activeportfirst;
  int activeportlast;
  int currentactiveport;
};
