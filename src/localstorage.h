#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>

#include "core/eventreceiver.h"
#include "core/types.h"
#include "path.h"
#include "localpathinfo.h"
#include "localfile.h"
#include "localstoragerequestdata.h"

class LocalTransfer;
class LocalDownload;
class LocalUpload;
class TransferMonitor;
class FTPConn;
class LocalFileList;

#define MAXREAD 524288

class LocalStorage : public Core::EventReceiver {
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
  Core::BinaryData getTempFileContent(const std::string &) const;
  Core::BinaryData getFileContent(const Path &) const;
  const Core::BinaryData & getStoreContent(int) const;
  void purgeStoreContent(int);
  int requestDelete(const Path & filename, bool care = false);
  bool deleteFile(const Path & filename);
  static bool deleteFileAbsolute(const Path & filename);
  static bool deleteRecursive(const Path & path);
  bool getDeleteResult(int requestid);
  static LocalPathInfo getPathInfo(const Path & path);
  static LocalPathInfo getPathInfo(const std::list<Path> & paths);
  int requestPathInfo(const Path & path);
  int requestPathInfo(const std::list<Path> & paths);
  LocalPathInfo getPathInfo(int requestid);
  static LocalFile getLocalFile(const Path & path);
  const Path & getTempPath() const;
  void setTempPath(const std::string &);
  void storeContent(int, const Core::BinaryData &);
  const Path & getDownloadPath() const;
  void setDownloadPath(const Path &);
  static std::shared_ptr<LocalFileList> getLocalFileList(const Path & path);
  int requestLocalFileList(const Path & path);
  bool requestReady(int requestid) const;
  std::shared_ptr<LocalFileList> getLocalFileList(int requestid);
  void asyncTaskComplete(int type, void * data);
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
  void executeAsyncRequest(LocalStorageRequestData * data);
private:
  void deleteRequestData(LocalStorageRequestData * reqdata);
  static LocalPathInfo getPathInfo(const Path & path, int currentdepth);
  std::map<int, Core::BinaryData> content;
  LocalDownload * getAvailableLocalDownload();
  LocalUpload * getAvailableLocalUpload();
  std::list<LocalDownload *> localdownloads;
  std::list<LocalUpload *> localuploads;
  std::map<int, LocalStorageRequestData *> readyrequests;
  Path temppath;
  Path downloadpath;
  int storeidcounter;
  bool useactivemodeaddress;
  std::string activemodeaddress;
  int activeportfirst;
  int activeportlast;
  int currentactiveport;
  int requestidcounter;
};
