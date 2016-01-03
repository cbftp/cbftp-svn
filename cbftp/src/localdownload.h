#pragma once

#include <fstream>
#include <string>

#include "localtransfer.h"

class TransferMonitor;
class FTPConn;
class LocalStorage;

class LocalDownload : public LocalTransfer {
public:
  LocalDownload(LocalStorage *);
  void engage(TransferMonitor *, std::string, std::string, std::string, int, bool, FTPConn *);
  void engage(TransferMonitor *, int, std::string, int, bool, FTPConn *);
  bool active() const;
  void FDConnected(int);
  void FDDisconnected(int);
  void FDData(int, char *, unsigned int);
  void FDSSLSuccess(int);
  void FDSSLFail(int);
  void FDFail(int, std::string);
  unsigned long long int size() const;
  int getStoreId() const;
private:
  void init(TransferMonitor *, FTPConn *, std::string, std::string, bool, int, bool);
  void append(char *, unsigned int);
  void openFile();
  TransferMonitor * tm;
  bool ssl;
  int sockid;
  bool inuse;
  bool inmemory;
  bool fileopened;
  int storeid;
  FTPConn * ftpconn;
  std::fstream filestream;
  std::string path;
  std::string filename;
  unsigned long long int filesize;
  char * buf;
  unsigned int buflen;
  unsigned int bufpos;
  LocalStorage * ls;
};
