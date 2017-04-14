#pragma once

#include "localtransfer.h"

class Path;

class LocalUpload : public LocalTransfer {
public:
  LocalUpload();
  void engage(TransferMonitor *, const Path &, const std::string &, const std::string &, int, bool, FTPConn *);
  bool engage(TransferMonitor *, const Path &, const std::string &, int, bool, FTPConn *);
  bool active() const;
  void FDConnected(int);
  void FDDisconnected(int);
  void FDData(int, char *, unsigned int);
  void FDSSLSuccess(int, const std::string &);
  void FDSSLFail(int);
  void FDSendComplete(int);
  void FDFail(int, const std::string &);
  unsigned long long int size() const;
private:
  void init(TransferMonitor *, FTPConn *, const Path &, const std::string &, bool, int, bool);
  void sendChunk();
  unsigned long long int filepos;
};
