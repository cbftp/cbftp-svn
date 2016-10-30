#pragma once

#include "localtransfer.h"

class LocalStorage;
class Path;

class LocalDownload : public LocalTransfer {
public:
  LocalDownload(LocalStorage *);
  void engage(TransferMonitor *, const Path &, const std::string &, const std::string &, int, bool, FTPConn *);
  bool engage(TransferMonitor *, const Path &, const std::string &, int, bool, FTPConn *);
  void engage(TransferMonitor *, int, const std::string &, int, bool, FTPConn *);
  bool engage(TransferMonitor *, int, int, bool, FTPConn *);
  void FDConnected(int);
  void FDDisconnected(int);
  void FDData(int, char *, unsigned int);
  void FDSSLSuccess(int);
  void FDSSLFail(int);
  void FDFail(int, std::string);
  unsigned long long int size() const;
  int getStoreId() const;
private:
  void init(TransferMonitor *, FTPConn *, const Path &, const std::string &, bool, int, bool, int, bool);
  void append(char *, unsigned int);
  int storeid;
  unsigned long long int filesize;
  unsigned int bufpos;
  LocalStorage * ls;
};
