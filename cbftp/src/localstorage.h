#pragma once

#include <string>
#include <list>
#include <map>

#include "core/pointer.h"
#include "core/types.h"
#include "path.h"

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
  void deleteFile(const Path &);
  const Path & getTempPath() const;
  void setTempPath(const std::string &);
  void storeContent(int, const BinaryData &);
  const Path & getDownloadPath() const;
  void setDownloadPath(const Path &);
  static Pointer<LocalFileList> getLocalFileList(const Path &);
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
