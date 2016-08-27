#pragma once

#include <string>
#include <list>
#include <map>

#include "core/pointer.h"
#include "core/types.h"

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
  LocalTransfer * passiveModeDownload(TransferMonitor *, const std::string &, const std::string &, const std::string &, int, bool, FTPConn *);
  LocalTransfer * passiveModeDownload(TransferMonitor *, const std::string &, int, bool, FTPConn *);
  LocalTransfer * passiveModeUpload(TransferMonitor *, const std::string &, const std::string &, const std::string &, int, bool, FTPConn *);
  LocalTransfer * activeModeDownload(TransferMonitor *, const std::string &, const std::string &, bool, FTPConn *);
  LocalTransfer * activeModeDownload(TransferMonitor *, bool, FTPConn *);
  LocalTransfer * activeModeUpload(TransferMonitor *, const std::string &, const std::string &, bool, FTPConn *);
  BinaryData getTempFileContent(const std::string &) const;
  BinaryData getFileContent(const std::string &) const;
  const BinaryData & getStoreContent(int) const;
  void purgeStoreContent(int);
  void deleteFile(std::string);
  std::string getTempPath() const;
  void setTempPath(const std::string &);
  void storeContent(int, const BinaryData &);
  std::string getDownloadPath() const;
  void setDownloadPath(const std::string &);
  static Pointer<LocalFileList> getLocalFileList(const std::string &);
  unsigned long long int getFileSize(const std::string &);
  bool getUseActiveModeAddress() const;
  std::string getActiveModeAddress() const;
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
  std::string temppath;
  std::string downloadpath;
  int storeidcounter;
  bool useactivemodeaddress;
  std::string activemodeaddress;
  int activeportfirst;
  int activeportlast;
  int currentactiveport;
};
