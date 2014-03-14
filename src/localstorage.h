#pragma once

#include <string>
#include <list>
#include <map>

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
  int passiveDownload(TransferMonitor *, std::string, bool, FTPConn *);
  int getFileContent(std::string, char *);
  int getStoreContent(int, char **);
  void purgeStoreContent(int);
  void deleteFile(std::string);
  std::string getTempPath();
  void setTempPath(std::string);
  void storeContent(int, char *, int);
private:
  std::map<int, std::pair<char *, int> > content;
  std::string getHostFromPASVString(std::string);
  int getPortFromPASVString(std::string);
  LocalTransfer * getAvailableLocalTransfer();
  std::list<LocalTransfer *> localtransfers;
  std::string temppath;
  int storeidcounter;
};
