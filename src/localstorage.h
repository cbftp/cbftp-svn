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
  LocalTransfer * passiveDownload(TransferMonitor *, std::string, std::string, bool, FTPConn *);
  int passiveDownload(TransferMonitor *, std::string, bool, FTPConn *);
  int getFileContent(std::string, char *) const;
  int getStoreContent(int, char **) const;
  void purgeStoreContent(int);
  void deleteFile(std::string);
  std::string getTempPath() const;
  void setTempPath(std::string);
  void storeContent(int, char *, int);
private:
  std::map<int, std::pair<char *, int> > content;
  std::string getHostFromPASVString(std::string) const;
  int getPortFromPASVString(std::string) const;
  LocalTransfer * getAvailableLocalTransfer();
  std::list<LocalTransfer *> localtransfers;
  std::string temppath;
  int storeidcounter;
};
