#pragma once

#include <string>
#include <list>

class LocalTransfer;
class TransferMonitor;
class FTPConn;

#define MAXREAD 524288

class LocalStorage {
public:
  LocalStorage();
  void readConfiguration();
  void writeState();
  void passiveDownload(TransferMonitor *, std::string, std::string, bool, FTPConn *);
  int getFileContent(std::string, char *);
  void deleteFile(std::string);
  std::string getTempPath();
  void setTempPath(std::string);
private:
  std::string getHostFromPASVString(std::string);
  int getPortFromPASVString(std::string);
  std::list<LocalTransfer *> localtransfers;
  std::string temppath;
};
